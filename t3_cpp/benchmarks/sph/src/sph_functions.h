#ifndef SPH_FUNCTIONS_H
#define SPH_FUNCTIONS_H

#include "sph_types.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <cstddef>

// ============================================================================
// SPATIAL HASHING FUNCTIONS
// ============================================================================

/// Returns a hash of the cell position
uint32_t getHash(const glm::ivec3 &cell);

/// Get the cell that the particle is in
glm::ivec3 getCell(Particle *p, float h);

/// Creates the particle neighbor hash table
uint32_t* createNeighborTable(Particle *sortedParticles, const size_t &particleCount);

// ============================================================================
// KERNEL FUNCTIONS
// ============================================================================

/// Calculate Poly6 kernel constant
float calculateKernelPoly6(float h);

/// Calculate Spiky gradient kernel constant
float calculateKernelSpikyGrad(float h);

/// Calculate Spiky Laplacian kernel constant
float calculateKernelSpikyLap(float h);

/// Calculate self density contribution
float calculateSelfDensity(float mass, float poly6, float h);

// ============================================================================
// DENSITY AND PRESSURE FUNCTIONS
// ============================================================================

/// Calculate density contribution from a single neighbor
float calculateDensityContribution(float dist2, float h2, float massPoly6);

/// Calculate pressure from density using equation of state
float calculatePressure(float density, float restDensity, float gasConstant);

/// Accumulate density from a neighbor particle
void accumulateDensityFromNeighbor(
    Particle* pi, Particle* pj, float massPoly6, float h2, float& density);

// ============================================================================
// FORCE CALCULATION FUNCTIONS
// ============================================================================

/// Calculate pressure force between two particles
glm::vec3 calculatePressureForce(
    Particle* pi, Particle* pj, float mass, float spikyGrad, float h);

/// Calculate viscosity force between two particles
glm::vec3 calculateViscosityForce(
    Particle* pi, Particle* pj, float mass, float viscosity, float spikyLap, float h);

/// Accumulate forces from a neighbor particle
void accumulateForcesFromNeighbor(
    Particle* pi, Particle* pj, float mass, float viscosity, 
    float spikyGrad, float spikyLap, float h, glm::vec3& force);

// ============================================================================
// INTEGRATION FUNCTIONS
// ============================================================================

/// Integrate velocity using semi-implicit Euler
void integrateVelocity(Particle* p, float deltaTime, float g);

/// Integrate position using semi-implicit Euler
void integratePosition(Particle* p, float deltaTime);

/// Handle ground collision
void handleGroundCollision(Particle* p, float h, float elasticity);

/// Handle wall collision in X direction
void handleWallCollisionX(Particle* p, float h, float boxWidth, float elasticity);

/// Handle wall collision in Z direction
void handleWallCollisionZ(Particle* p, float h, float boxWidth, float elasticity);

/// Apply all boundary conditions
void applyBoundaryConditions(Particle* p, float h, float boxWidth, float elasticity);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/// Check if two particles are within smoothing radius
bool isWithinSmoothingRadius(glm::vec3 pos1, glm::vec3 pos2, float h2);

/// Calculate squared distance between two positions
float calculateDistanceSquared(glm::vec3 p1, glm::vec3 p2);

/// Normalize direction vector safely
glm::vec3 normalizeDirection(glm::vec3 vec);

/// Compute acceleration from force and density
glm::vec3 computeAcceleration(glm::vec3 force, float density, float g);

// ============================================================================
// PARALLEL COMPUTATION FUNCTIONS
// ============================================================================

/// Calculate hashes for a range of particles
void parallelCalculateHashes(
    Particle *particles, size_t start, size_t end, float h);

/// Calculate densities and pressures for a range of particles
void parallelDensityAndPressures(
    Particle *particles, const size_t particleCount, const size_t start,
    const size_t end, const uint32_t *particleTable,
    float mass, float poly6, float h, float h2, float selfDens,
    float restDensity, float gasConstant);

/// Calculate forces for a range of particles
void parallelForces(
    Particle *particles, const size_t particleCount, const size_t start,
    const size_t end, const uint32_t *particleTable,
    float mass, float viscosity, float spikyGrad, float spikyLap, float h, float h2);

/// Update particle positions for a range of particles
void parallelUpdateParticlePositions(
    Particle *particles, const size_t particleCount, const size_t start,
    const size_t end, float h, float boxWidth, float elasticity,
    float g, float deltaTime);

#endif // SPH_FUNCTIONS_H
