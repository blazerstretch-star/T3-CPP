CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I./src

SRC = src/jemalloc_utils_complete.cpp
TESTS = $(wildcard tests/*.cpp)
TARGETS = $(patsubst tests/%.cpp,%,$(TESTS))

all: $(TARGETS)

%: tests/%.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC)

clean:
	rm -f $(TARGETS)

test: all
	@passed=0; failed=0; \
	for test in $(TARGETS); do \
		echo "Running $$test..."; \
		if ./$$test 2>&1; then \
			echo "✓ $$test passed"; \
			passed=$$((passed + 1)); \
		else \
			echo "✗ $$test failed"; \
			failed=$$((failed + 1)); \
		fi; \
	done; \
	echo ""; \
	echo "Results: $$passed passed, $$failed failed"

.PHONY: all clean test
