#!/usr/bin/env python3
"""
Hybrid BM25 + Dense Embedding Retriever using vLLM endpoints
Optimized for multi-GPU vLLM deployment (ports 8000-8003)

Architecture:
1. Stage 1: BM25 retrieval (lexical, high recall)
2. Stage 2: Dense reranking (semantic, high precision)
3. Fusion: Reciprocal Rank Fusion (RRF)
4. Leakage prevention: Target function masking + file exclusion

Based on SOTA research (2024):
- CoIR benchmark
- SFR-Embedding-Code (Salesforce)
- RACG survey
"""

import os
import re
import json
import pickle
import requests
from pathlib import Path
from typing import List, Tuple, Optional, Dict
import numpy as np
from rank_bm25 import BM25Okapi
import time


class IdentifierAwareTokenizer:
    """Tokenize code with identifier-aware splitting
    
    Handles:
    - CamelCase: calculateDistance → [calculate, distance]
    - snake_case: calculate_distance → [calculate, distance]
    - Namespaces: std::vector → [std, vector]
    - Templates: vector<int> → [vector, int]
    """
    
    @staticmethod
    def tokenize(text: str) -> List[str]:
        """Split code into identifier-aware tokens"""
        tokens = []
        
        # Extract identifiers and keywords
        raw_tokens = re.findall(
            r'[a-zA-Z_][a-zA-Z0-9_]*(?:::[a-zA-Z_][a-zA-Z0-9_]*)*|[0-9]+',
            text
        )
        
        for token in raw_tokens:
            if len(token) <= 1:
                continue
            
            # Handle namespace separators
            if '::' in token:
                parts = token.split('::')
                tokens.extend([p.lower() for p in parts if len(p) > 1])
                tokens.append(token.lower())
                continue
            
            # Split CamelCase with improved handling of numbers
            # Pattern: Insert space before uppercase letters and before/after digit sequences
            # Point3D → Point 3D → [Point, 3D]
            # calculateDistance → calculate Distance → [calculate, Distance]
            spaced = re.sub(r'([a-z])([A-Z])', r'\1 \2', token)  # lowercase before uppercase
            spaced = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1 \2', spaced)  # Multiple uppercase before lowercase
            spaced = re.sub(r'([a-zA-Z])([0-9])', r'\1 \2', spaced)  # Letter before digit
            spaced = re.sub(r'([0-9])([a-zA-Z])', r'\1 \2', spaced)  # Digit before letter
            
            camel_parts = spaced.split()
            if len(camel_parts) > 1:
                # Merge single digits with following single letters (3 D → 3D)
                merged_parts = []
                i = 0
                while i < len(camel_parts):
                    part = camel_parts[i]
                    # If current is single digit and next is single letter, merge them
                    if (i + 1 < len(camel_parts) and 
                        len(part) == 1 and part.isdigit() and 
                        len(camel_parts[i + 1]) == 1 and camel_parts[i + 1].isalpha()):
                        merged_parts.append(part + camel_parts[i + 1])
                        i += 2
                    else:
                        merged_parts.append(part)
                        i += 1
                
                tokens.extend([p.lower() for p in merged_parts if len(p) > 1])
            
            # Split snake_case
            if '_' in token:
                snake_parts = token.split('_')
                tokens.extend([p.lower() for p in snake_parts if len(p) > 1])
            
            # Keep original
            tokens.append(token.lower())
        
        return tokens


class VLLMEmbeddingClient:
    """Client for vLLM embedding endpoints with load balancing"""
    
    def __init__(self, base_ports: List[int] = [8000, 8001, 8002, 8003], 
                 host: str = "localhost", timeout: int = 300,
                 model_name: str = "Qwen/Qwen3-Embedding-8B",
                 embedding_dim: int = 4096):
        """
        Args:
            base_ports: List of vLLM ports (one per GPU)
            host: vLLM host
            timeout: Request timeout in seconds
            model_name: Embedding model name (default: Qwen3-Embedding-8B)
            embedding_dim: Output embedding dimension (32-4096 for Qwen3)
        """
        self.endpoints = [f"http://{host}:{port}/v1/embeddings" for port in base_ports]
        self.timeout = timeout
        self.model_name = model_name
        self.embedding_dim = embedding_dim
        self.current_endpoint_idx = 0
        
        print(f"  🔗 vLLM endpoints: {self.endpoints}")
        print(f"  🤖 Model: {model_name}")
        print(f"  📐 Embedding dimension: {embedding_dim}")
        
        # Test connectivity
        self._test_endpoints()
    
    def _test_endpoints(self):
        """Test which endpoints are available"""
        available = []
        for endpoint in self.endpoints:
            try:
                # Quick health check with minimal text
                response = requests.post(
                    endpoint,
                    json={"input": "test", "model": self.model_name},
                    timeout=5
                )
                if response.status_code == 200:
                    available.append(endpoint)
                    print(f"    ✓ {endpoint} - OK")
                else:
                    print(f"    ✗ {endpoint} - Status {response.status_code}")
            except Exception as e:
                print(f"    ✗ {endpoint} - {str(e)[:50]}")
        
        if not available:
            raise RuntimeError("No vLLM endpoints available!")
        
        self.endpoints = available
        print(f"  ✓ {len(self.endpoints)} endpoints available")
    
    def _get_next_endpoint(self) -> str:
        """Round-robin load balancing"""
        endpoint = self.endpoints[self.current_endpoint_idx]
        self.current_endpoint_idx = (self.current_endpoint_idx + 1) % len(self.endpoints)
        return endpoint
    
    def encode(self, texts: List[str], show_progress: bool = True, batch_size: int = 32) -> np.ndarray:
        """Encode texts using vLLM endpoints with batching and parallelism
        
        Args:
            texts: List of texts to encode
            show_progress: Show progress bar
            batch_size: Batch size per GPU (default: 32)
        
        Returns:
            numpy array of embeddings (n_texts, embedding_dim)
        """
        if isinstance(texts, str):
            texts = [texts]
        
        if len(texts) == 0:
            return np.array([])
        
        # For small batches, use simple sequential processing
        if len(texts) <= 4:
            return self._encode_sequential(texts, show_progress)
        
        # For large batches, use parallel batching across GPUs
        return self._encode_parallel_batched(texts, show_progress, batch_size)
    def _encode_sequential(self, texts: List[str], show_progress: bool) -> np.ndarray:
        """Sequential encoding (for small batches)"""
        embeddings = []
        
        for i, text in enumerate(texts):
            if show_progress and (i + 1) % 10 == 0:
                print(f"    Encoded {i+1}/{len(texts)} texts...")
            
            # Truncate text if too long (32K token limit for Qwen3 ≈ 128K chars, use 64K for safety)
            MAX_CHARS = 64000
            if len(text) > MAX_CHARS:
                print(f"    ⚠️  Text {i} too long ({len(text)} chars), truncating to {MAX_CHARS}")
                text = text[:MAX_CHARS]
            
            # Get endpoint (round-robin)
            endpoint = self._get_next_endpoint()
            
            # Call vLLM API
            try:
                response = requests.post(
                    endpoint,
                    json={
                        "input": text,
                        "model": self.model_name,
                        "encoding_format": "float"
                        # Note: Qwen3-Embedding-8B doesn't support 'dimensions' parameter
                    },
                    timeout=self.timeout
                )
                
                if response.status_code != 200:
                    print(f"    ⚠️  API error {response.status_code}: {response.text[:100]}")
                    # Retry with next endpoint
                    endpoint = self._get_next_endpoint()
                    response = requests.post(
                        endpoint,
                        json={
                            "input": text,
                            "model": self.model_name,
                            "encoding_format": "float"
                            # Note: Qwen3-Embedding-8B doesn't support 'dimensions' parameter
                        },
                        timeout=self.timeout
                    )
                
                result = response.json()
                embedding = result['data'][0]['embedding']
                embeddings.append(embedding)
                
            except Exception as e:
                print(f"    ❌ Encoding failed for text {i}: {e}")
                # Use zero vector as fallback
                embeddings.append([0.0] * self.embedding_dim)
        
        return np.array(embeddings)
    
    def _encode_batch(self, endpoint: str, texts: List[str]) -> List[List[float]]:
        """Encode a batch of texts on a single endpoint
        
        Args:
            endpoint: vLLM endpoint URL
            texts: List of texts to encode
        
        Returns:
            List of embeddings
        """
        # Truncate texts if needed
        MAX_CHARS = 64000
        truncated_texts = []
        for text in texts:
            if len(text) > MAX_CHARS:
                truncated_texts.append(text[:MAX_CHARS])
            else:
                truncated_texts.append(text)
        
        try:
            response = requests.post(
                endpoint,
                json={
                    "input": truncated_texts,  # Send batch
                    "model": self.model_name,
                    "encoding_format": "float"
                    # Note: Qwen3-Embedding-8B doesn't support 'dimensions' parameter
                },
                timeout=self.timeout
            )
            
            if response.status_code != 200:
                raise RuntimeError(f"API error {response.status_code}: {response.text[:200]}")
            
            result = response.json()
            return [item['embedding'] for item in result['data']]
        
        except Exception as e:
            print(f"    ❌ Batch encoding failed on {endpoint}: {e}")
            # Return zero vectors as fallback
            return [[0.0] * self.embedding_dim for _ in texts]
    
    def _encode_parallel_batched(self, texts: List[str], show_progress: bool, batch_size: int) -> np.ndarray:
        """Encode texts using parallel batching across all GPUs
        
        Strategy:
        1. Split texts into batches
        2. Distribute batches across GPUs
        3. Send requests in parallel using ThreadPoolExecutor
        4. Combine results
        
        Args:
            texts: List of texts to encode
            show_progress: Show progress
            batch_size: Batch size per GPU (auto-adjusted for optimal GPU utilization)
        
        Returns:
            numpy array of embeddings
        """
        from concurrent.futures import ThreadPoolExecutor, as_completed
        
        num_gpus = len(self.endpoints)
        
        # OPTIMIZATION: Adaptive batch sizing for optimal GPU utilization
        # If we have fewer texts than batch_size, reduce batch_size to use all GPUs
        if len(texts) < batch_size:
            # Distribute evenly across GPUs
            batch_size = max(1, len(texts) // num_gpus)
            if show_progress:
                print(f"    💡 Adaptive batching: reduced batch_size to {batch_size} for {len(texts)} texts")
        
        if show_progress:
            print(f"    🚀 Parallel batching: {len(texts)} texts across {num_gpus} GPUs (batch_size={batch_size})")
        
        # Split texts into batches
        batches = []
        for i in range(0, len(texts), batch_size):
            batch = texts[i:i + batch_size]
            batches.append(batch)
        
        if show_progress:
            print(f"    📦 Created {len(batches)} batches (will use {min(len(batches), num_gpus)} GPUs)")
        
        # Distribute batches across GPUs using round-robin
        gpu_batches = [[] for _ in range(num_gpus)]
        for i, batch in enumerate(batches):
            gpu_id = i % num_gpus
            gpu_batches[gpu_id].append(batch)
        
        # Process batches in parallel
        all_embeddings = [None] * len(texts)  # Preserve order
        completed_count = 0
        
        with ThreadPoolExecutor(max_workers=num_gpus) as executor:
            # Submit all GPU tasks
            futures = []
            for gpu_id, batches_for_gpu in enumerate(gpu_batches):
                if not batches_for_gpu:
                    continue
                
                endpoint = self.endpoints[gpu_id]
                for batch_idx, batch in enumerate(batches_for_gpu):
                    # Calculate global batch index for ordering
                    global_batch_idx = gpu_id + batch_idx * num_gpus
                    future = executor.submit(self._encode_batch, endpoint, batch)
                    futures.append((future, global_batch_idx, len(batch)))
            
            # Collect results as they complete
            for future, global_batch_idx, batch_len in futures:
                try:
                    embeddings = future.result()
                    
                    # Insert embeddings in correct position
                    start_idx = global_batch_idx * batch_size
                    for i, emb in enumerate(embeddings):
                        if start_idx + i < len(texts):
                            all_embeddings[start_idx + i] = emb
                    
                    completed_count += batch_len
                    if show_progress:
                        print(f"    ✓ Progress: {completed_count}/{len(texts)} texts encoded")
                
                except Exception as e:
                    print(f"    ❌ Batch failed: {e}")
                    # Fill with zero vectors
                    start_idx = global_batch_idx * batch_size
                    for i in range(batch_len):
                        if start_idx + i < len(texts):
                            all_embeddings[start_idx + i] = [0.0] * self.embedding_dim
        
        # Filter out None values (shouldn't happen, but safety check)
        all_embeddings = [emb if emb is not None else [0.0] * self.embedding_dim 
                         for emb in all_embeddings]
        
        return np.array(all_embeddings)


class HybridCodeRetriever:
    """Hybrid BM25 + Dense Embedding Retriever using vLLM endpoints"""
    
    def __init__(
        self,
        repo_path: str,
        mask_function_id: Optional[str] = None,  # DEPRECATED: Now masks ALL functions automatically
        vllm_ports: List[int] = [8000, 8001, 8002, 8003],
        vllm_host: str = "localhost",
        file_extensions: List[str] = None,
        cache_dir: Optional[str] = None,
        chunk_size: int = 112000,  # ~28K tokens (87% of Qwen3's 32K context)
        chunk_overlap: int = 8000,   # ~2K tokens overlap
        embedding_model: str = "Qwen/Qwen3-Embedding-8B",
        embedding_dim: int = 4096
    ):
        """
        Args:
            repo_path: Path to repository
            mask_function_id: DEPRECATED - Now masks ALL functions automatically (kept for compatibility)
            vllm_ports: List of vLLM ports (one per GPU)
            vllm_host: vLLM host
            file_extensions: File types to index
            cache_dir: Directory to cache embeddings
            chunk_size: Max characters per chunk (default: 112K for Qwen3's 32K context)
            chunk_overlap: Overlap between chunks (default: 8K)
            embedding_model: Model name (default: Qwen3-Embedding-8B)
            embedding_dim: Output dimension (32-4096 for Qwen3)
        """
        self.repo_path = Path(repo_path)
        # CRITICAL: Only index context/ folder (same as agentic tools)
        self.context_path = self.repo_path / 'context'
        
        # Strict enforcement: context/ MUST exist
        if not self.context_path.exists():
            raise ValueError(
                f"Missing required 'context/' directory in repository: {self.repo_path}.\n"
                "Ensure benchmarks/<repo>/context exists and contains the allowed files."
            )
        
        print(f"  ✓ Context folder exists: {self.context_path}")
        
        self.mask_function_id = mask_function_id
        # Include all common C/C++ extensions (case-insensitive check will be done)
        self.file_extensions = file_extensions or [
            '.h', '.hh', '.hpp', '.hxx', '.h++',
            '.c', '.cc', '.cpp', '.cxx', '.c++', '.cp',
            '.tpp', '.ipp', '.inl', '.ixx', '.cppm'
        ]
        self.cache_dir = Path(cache_dir) if cache_dir else self.repo_path / '.retrieval_cache'
        self.cache_dir.mkdir(exist_ok=True)
        
        # Chunking parameters
        self.chunk_size = chunk_size
        self.chunk_overlap = chunk_overlap
        self.embedding_model = embedding_model
        self.embedding_dim = embedding_dim
        
        # Initialize tokenizer
        self.tokenizer = IdentifierAwareTokenizer()
        
        # Initialize vLLM client
        self.embedding_client = VLLMEmbeddingClient(
            base_ports=vllm_ports,
            host=vllm_host,
            model_name=embedding_model,
            embedding_dim=embedding_dim
        )
        
        # Storage
        self.corpus = []  # List of text chunks
        self.file_paths = []  # List of (file_path, chunk_id) tuples
        self.file_embeddings = None
        self.bm25 = None
        
        # Build index
        self._build_index()
    
    def _mask_function_in_content(self, content: str, file_path: Path) -> str:
        """Mask ALL function implementations to prevent leakage
        
        Uses the same pattern as agentic_tools.py:
        Masks ALL functions marked with FUNCTION_ID comments
        // FUNCTION_ID: <id> - START
        ...
        // FUNCTION_ID: <id> - END
        """
        # Mask ALL functions marked with FUNCTION_ID comments (same as agentic_tools.py)
        pattern = r'// FUNCTION_ID: ([^\n]+) - START.*?// FUNCTION_ID: \1 - END'
        replacement = r'// FUNCTION_ID: \1 - START\n  // [MASKED FOR NO LEAKAGE]\n  // FUNCTION_ID: \1 - END'
        
        masked_content = re.sub(pattern, replacement, content, flags=re.DOTALL)
        
        # Count how many functions were masked
        num_masked = len(re.findall(r'// FUNCTION_ID: ([^\n]+) - START', content))
        if num_masked > 0:
            print(f"    🔒 Masked {num_masked} function(s) in {file_path.name}")
        
        return masked_content
    
    def _chunk_content(self, content: str, file_path: Path) -> List[str]:
        """Split content into chunks with overlap for better retrieval
        
        Args:
            content: File content to chunk
            file_path: Path to file (for logging)
        
        Returns:
            List of text chunks
        """
        # If content is small enough, return as single chunk
        if len(content) <= self.chunk_size:
            return [content]
        
        chunks = []
        start = 0
        chunk_count = 0
        
        while start < len(content):
            end = start + self.chunk_size
            
            # Try to break at a newline near the end for cleaner chunks
            if end < len(content):
                # Look for newline in last 200 chars of chunk
                newline_pos = content.rfind('\n', end - 200, end)
                if newline_pos != -1:
                    end = newline_pos + 1
            
            chunk = content[start:end]
            
            # Add chunk metadata for debugging
            chunk_header = f"// [Chunk {chunk_count + 1} of {file_path.name}]\n"
            chunks.append(chunk_header + chunk)
            
            chunk_count += 1
            
            # Move start position with overlap
            start = end - self.chunk_overlap
            
            # Safety: prevent infinite loop
            if start >= len(content) or chunk_count > 100:
                break
        
        if chunk_count > 1:
            print(f"    📄 Chunked {file_path.name}: {len(content)} chars → {chunk_count} chunks")
        
        return chunks
    
    def _build_index(self):
        """Build BM25 and dense embedding indices (ONLY from context/ folder)"""
        print(f"  🔍 Indexing context folder: {self.context_path}")
        print(f"  🔒 Masking ALL functions with FUNCTION_ID markers")
        
        # Check cache
        cache_file = self.cache_dir / f"index_{self.mask_function_id or 'all'}.pkl"
        if cache_file.exists():
            print(f"  📦 Loading cached index: {cache_file.name}")
            try:
                with open(cache_file, 'rb') as f:
                    cache_data = pickle.load(f)
                self.corpus = cache_data['corpus']
                self.file_paths = cache_data['file_paths']
                self.file_embeddings = cache_data['embeddings']
                self.bm25 = cache_data['bm25']
                print(f"  ✓ Loaded {len(self.corpus)} files from cache")
                return
            except Exception as e:
                print(f"  ⚠️  Cache load failed: {e}, rebuilding...")
        
        # CRITICAL: Collect files ONLY from context/ folder (prevent seeing answers in src/)
        for root, dirs, files in os.walk(self.context_path):
            # Security: Prevent directory traversal (ensure we're still under context_path)
            root_abs = os.path.abspath(root)
            context_abs = os.path.abspath(str(self.context_path))
            if not root_abs.startswith(context_abs):
                continue
            
            dirs[:] = [d for d in dirs if d not in ['build', 'test', 'tests', '.git', '__pycache__', '.retrieval_cache']]
            
            for file in files:
                # Case-insensitive extension check
                file_lower = file.lower()
                if any(file_lower.endswith(ext) for ext in self.file_extensions):
                    file_path = Path(root) / file
                    
                    try:
                        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                        
                        # Skip truly empty files, but keep files with just whitespace/comments
                        if len(content.strip()) < 10:  # Less than 10 non-whitespace chars
                            continue
                        
                        content = self._mask_function_in_content(content, file_path)
                        
                        # CRITICAL: Chunk large files for better retrieval
                        # Qwen3-Embedding-8B: 32K token limit ≈ 128K chars (use 112K chunks = ~28K tokens for safety)
                        chunks = self._chunk_content(content, file_path)
                        
                        for chunk_id, chunk_text in enumerate(chunks):
                            self.corpus.append(chunk_text)
                            self.file_paths.append((file_path, chunk_id))
                    
                    except Exception as e:
                        print(f"    ⚠️  Skipping {file_path.name}: {e}")
        
        print(f"  ✓ Collected {len(self.corpus)} files")
        
        # Build BM25 index
        print(f"  🔨 Building BM25 index...")
        tokenized_corpus = [self.tokenizer.tokenize(doc) for doc in self.corpus]
        self.bm25 = BM25Okapi(tokenized_corpus)
        print(f"  ✓ BM25 index built")
        
        # Build dense embeddings using vLLM
        print(f"  🔨 Building dense embeddings via vLLM...")
        self.file_embeddings = self.embedding_client.encode(
            self.corpus,
            show_progress=True
        )
        print(f"  ✓ Dense embeddings built: {self.file_embeddings.shape}")
        
        # Cache index
        print(f"  💾 Caching index...")
        try:
            with open(cache_file, 'wb') as f:
                pickle.dump({
                    'corpus': self.corpus,
                    'file_paths': self.file_paths,
                    'embeddings': self.file_embeddings,
                    'bm25': self.bm25
                }, f)
            print(f"  ✓ Index cached to {cache_file.name}")
        except Exception as e:
            print(f"  ⚠️  Cache save failed: {e}")
    
    def _reciprocal_rank_fusion(
        self,
        bm25_scores: np.ndarray,
        dense_scores: np.ndarray,
        k: int = 60
    ) -> np.ndarray:
        """Combine BM25 and dense scores using Reciprocal Rank Fusion"""
        bm25_ranks = np.argsort(-bm25_scores)
        dense_ranks = np.argsort(-dense_scores)
        
        bm25_rank_array = np.empty_like(bm25_ranks)
        bm25_rank_array[bm25_ranks] = np.arange(len(bm25_ranks))
        
        dense_rank_array = np.empty_like(dense_ranks)
        dense_rank_array[dense_ranks] = np.arange(len(dense_ranks))
        
        rrf_scores = (1.0 / (k + bm25_rank_array)) + (1.0 / (k + dense_rank_array))
        
        return rrf_scores
    
    def retrieve(
        self,
        query: str,
        top_k: int = 5,
        stage1_candidates: int = 100,  # Increased from 50 for more diversity
        max_chars_per_file: int = 5000,
        exclude_target_file: bool = True,
        fusion_method: str = 'rrf',
        file_type_weights: Dict[str, float] = None,
        exclude_test_files: bool = False,
        min_code_ratio: float = 0.0
    ) -> List[Tuple[Path, str, float, Dict[str, float]]]:
        """Hybrid retrieval: BM25 + Dense reranking with smart filtering
        
        Args:
            query: Search query
            top_k: Number of results to return
            stage1_candidates: Number of BM25 candidates (increased for diversity)
            max_chars_per_file: Max chars per result
            exclude_target_file: Exclude file containing target function
            fusion_method: 'rrf' or 'weighted'
            file_type_weights: Boost scores by file type (e.g., {'h': 1.2, 'cpp': 1.5, 'test': 0.5})
            exclude_test_files: Completely exclude test files
            min_code_ratio: Minimum ratio of code to comments (0.0-1.0)
        
        Returns:
            List of (file_path, content, score, breakdown)
        """
        
        if not self.bm25 or self.file_embeddings is None:
            return []
        
        # Default file type weights (boost implementation, lower test files)
        if file_type_weights is None:
            file_type_weights = {
                'h': 1.2,      # Headers: slight boost
                'hpp': 1.2,
                'hxx': 1.2,
                'cpp': 1.5,    # Implementation: strong boost
                'cc': 1.5,
                'cxx': 1.5,
                'test': 0.5,   # Tests: keep but rank lower
            }
        
        # Stage 1: BM25 retrieval
        tokenized_query = self.tokenizer.tokenize(query)
        bm25_scores = self.bm25.get_scores(tokenized_query)
        
        top_indices = np.argsort(-bm25_scores)[:stage1_candidates]
        
        # Stage 2: Dense reranking via vLLM
        query_embedding = self.embedding_client.encode([query], show_progress=False)[0]
        
        candidate_embeddings = self.file_embeddings[top_indices]
        dense_scores_candidates = np.dot(candidate_embeddings, query_embedding)
        
        dense_scores = np.zeros(len(self.corpus))
        dense_scores[top_indices] = dense_scores_candidates
        
        # Fusion
        if fusion_method == 'rrf':
            final_scores = self._reciprocal_rank_fusion(bm25_scores, dense_scores)
        else:
            bm25_norm = (bm25_scores - bm25_scores.min()) / (bm25_scores.max() - bm25_scores.min() + 1e-8)
            dense_norm = (dense_scores - dense_scores.min()) / (dense_scores.max() - dense_scores.min() + 1e-8)
            final_scores = 0.5 * bm25_norm + 0.5 * dense_norm
        
        # Apply file type boosting
        boosted_scores = final_scores.copy()
        for idx in range(len(self.corpus)):
            file_path, _ = self.file_paths[idx]
            file_name_lower = file_path.name.lower()
            
            # Check file type and apply weight
            for file_type, weight in file_type_weights.items():
                if file_type == 'test':
                    # Test file detection
                    if 'test' in file_name_lower or 'test' in str(file_path).lower():
                        boosted_scores[idx] *= weight
                        break
                elif file_name_lower.endswith(f'.{file_type}'):
                    boosted_scores[idx] *= weight
                    break
        
        ranked_indices = np.argsort(-boosted_scores)
        
        # Find target file (any file with FUNCTION_ID markers)
        target_file_path = None
        if exclude_target_file:
            for idx in ranked_indices:
                file_path, chunk_id = self.file_paths[idx]
                # Check for any FUNCTION_ID marker in the file
                if "FUNCTION_ID:" in self.corpus[idx]:
                    target_file_path = file_path
                    print(f"    🚫 Excluding target file: {target_file_path.name}")
                    break
        
        # Build results with smart filtering
        results = []
        seen_files = set()  # Track files we've already added
        
        for idx in ranked_indices:
            file_path, chunk_id = self.file_paths[idx]
            
            # Skip if we already have top_k unique files
            if len(results) >= top_k:
                break
            
            # Skip target file
            if target_file_path and file_path == target_file_path:
                continue
            
            # Skip if we already added this file (take only best chunk per file)
            if file_path in seen_files:
                continue
            
            # Filter: Exclude test files if requested
            if exclude_test_files:
                file_name_lower = file_path.name.lower()
                if 'test' in file_name_lower or 'test' in str(file_path).lower():
                    continue
            
            # Get chunk content
            chunk_content = self.corpus[idx]
            
            # Filter: Check code ratio (exclude files with too many comments/license)
            if min_code_ratio > 0.0:
                code_ratio = self._calculate_code_ratio(chunk_content)
                if code_ratio < min_code_ratio:
                    continue
            
            seen_files.add(file_path)
            
            # Remove chunk header for cleaner output
            chunk_content = re.sub(r'^// \[Chunk \d+ of [^\]]+\]\n', '', chunk_content)
            
            if len(chunk_content) > max_chars_per_file:
                chunk_content = chunk_content[:max_chars_per_file] + "\n\n// ... (truncated)"
            
            score_breakdown = {
                'bm25': float(bm25_scores[idx]),
                'dense': float(dense_scores[idx]),
                'final': float(boosted_scores[idx]),  # Use boosted score
                'chunk_id': chunk_id
            }
            
            results.append((file_path, chunk_content, boosted_scores[idx], score_breakdown))
        
        return results
    
    def _calculate_code_ratio(self, content: str) -> float:
        """Calculate ratio of code lines to total lines
        
        Filters out:
        - Empty lines
        - Comment-only lines (// or /* */)
        - License headers
        
        Returns:
            Ratio between 0.0 and 1.0
        """
        lines = content.split('\n')
        if not lines:
            return 0.0
        
        code_lines = 0
        in_block_comment = False
        license_keywords = ['license', 'copyright', 'permission', 'warranty']
        
        for line in lines:
            stripped = line.strip()
            
            # Skip empty lines
            if not stripped:
                continue
            
            # Track block comments
            if '/*' in stripped:
                in_block_comment = True
            if '*/' in stripped:
                in_block_comment = False
                continue
            
            # Skip if in block comment
            if in_block_comment:
                continue
            
            # Skip single-line comments
            if stripped.startswith('//'):
                # But check if it's a license header
                if any(kw in stripped.lower() for kw in license_keywords):
                    continue
                continue
            
            # This is a code line
            code_lines += 1
        
        return code_lines / len(lines) if lines else 0.0
    
    def retrieve_with_query_expansion(
        self,
        task: dict,
        top_k: int = 5,
        exclude_target_file: bool = True
    ) -> List[Tuple[Path, str, float, Dict[str, float]]]:
        """Retrieve using expanded query from task metadata"""
        
        query_parts = []
        
        if 'signature' in task:
            query_parts.append(task['signature'])
        
        if 'namespace' in task:
            namespace = task['namespace']
            if '::' in namespace:
                class_part = namespace.rsplit('::', 1)[0]
                query_parts.append(class_part)
        
        if 'requirement' in task:
            req = task['requirement']
            if isinstance(req, dict):
                if 'Functionality' in req:
                    query_parts.append(req['Functionality'])
                if 'Arguments' in req:
                    query_parts.append(req['Arguments'])
        
        query = " ".join(query_parts)
        
        return self.retrieve(query, top_k=top_k, exclude_target_file=exclude_target_file)
