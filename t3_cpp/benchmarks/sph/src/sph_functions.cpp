#include "sph_functions.h"
#include <glm/gtx/norm.hpp>
#include <cmath>
#include <algorithm>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


// ============================================================================
// SPATIAL HASHING FUNCTIONS
// ============================================================================

uint32_t getHash(const glm::ivec3 &cell) {
// FUNCTION_ID: sph_func001 - START
    return (
        (uint)(cell.x * 73856093)
        ^ (uint)(cell.y * 19349663)
        ^ (uint)(cell.z * 83492791)
    ) % TABLE_SIZE;
// FUNCTION_ID: sph_func001 - END
}

glm::ivec3 getCell(Particle *p, float h) {
// FUNCTION_ID: sph_func002 - START
    return glm::ivec3(p->position.x / h, p->position.y / h, p->position.z / h);
// FUNCTION_ID: sph_func002 - END
}

uint32_t* createNeighborTable(Particle *sortedParticles, const size_t &particleCount) {
// FUNCTION_ID: sph_func003 - START
    uint32_t *particleTable = (uint32_t *)malloc(sizeof(uint32_t) * TABLE_SIZE);
    for (size_t i = 0; i < TABLE_SIZE; ++i) {
        particleTable[i] = NO_PARTICLE;
    }

    uint32_t prevHash = NO_PARTICLE;
    for (size_t i = 0; i < particleCount; ++i) {
        uint32_t currentHash = sortedParticles[i].hash;
        if (currentHash != prevHash) {
            particleTable[currentHash] = i;
            prevHash = currentHash;
        }
    }
    return particleTable;
// FUNCTION_ID: sph_func003 - END
}

// ============================================================================
// KERNEL FUNCTIONS
// ============================================================================

float calculateKernelPoly6(float h) {
// FUNCTION_ID: sph_func004 - START
    return 315.0f / (64.0f * M_PI * std::pow(h, 9));
// FUNCTION_ID: sph_func004 - END
}

float calculateKernelSpikyGrad(float h) {
// FUNCTION_ID: sph_func005 - START
    return -45.0f / (M_PI * std::pow(h, 6));
// FUNCTION_ID: sph_func005 - END
}

float calculateKernelSpikyLap(float h) {
// FUNCTION_ID: sph_func006 - START
    return 45.0f / (M_PI * std::pow(h, 6));
// FUNCTION_ID: sph_func006 - END
}

float calculateSelfDensity(float mass, float poly6, float h) {
// FUNCTION_ID: sph_func007 - START
    return mass * poly6 * std::pow(h * h, 3);
// FUNCTION_ID: sph_func007 - END
}

// ============================================================================
// DENSITY AND PRESSURE FUNCTIONS
// ============================================================================

float calculateDensityContribution(float dist2, float h2, float massPoly6) {
// FUNCTION_ID: sph_func008 - START
    return massPoly6 * std::pow(h2 - dist2, 3);
// FUNCTION_ID: sph_func008 - END
}

float calculatePressure(float density, float restDensity, float gasConstant) {
// FUNCTION_ID: sph_func009 - START
    return gasConstant * (density - restDensity);
// FUNCTION_ID: sph_func009 - END
}

void accumulateDensityFromNeighbor(Particle* pi, Particle* pj, float massPoly6, float h2, float& density) {
// FUNCTION_ID: sph_func010 - START
    float dist2 = glm::length2(pj->position - pi->position);
    if (dist2 < h2) {
        density += massPoly6 * std::pow(h2 - dist2, 3);
    }
// FUNCTION_ID: sph_func010 - END
}

// ============================================================================
// FORCE CALCULATION FUNCTIONS
// ============================================================================

glm::vec3 calculatePressureForce(Particle* pi, Particle* pj, float mass, float spikyGrad, float h) {
// FUNCTION_ID: sph_func011 - START
    float dist2 = glm::length2(pj->position - pi->position);
    if (dist2 >= h * h) {
        return glm::vec3(0);
    }
    
    float dist = std::sqrt(dist2);
    glm::vec3 dir = glm::normalize(pj->position - pi->position);
    
    glm::vec3 pressureForce = -dir * mass * (pi->pressure + pj->pressure) 
                              / (2 * pj->density) * spikyGrad;
    pressureForce *= std::pow(h - dist, 2);
    
    return pressureForce;
// FUNCTION_ID: sph_func011 - END
}

glm::vec3 calculateViscosityForce(Particle* pi, Particle* pj, float mass, float viscosity, float spikyLap, float h) {
// FUNCTION_ID: sph_func012 - START
    float dist2 = glm::length2(pj->position - pi->position);
    if (dist2 >= h * h) {
        return glm::vec3(0);
    }
    
    float dist = std::sqrt(dist2);
    glm::vec3 velocityDif = pj->velocity - pi->velocity;
    glm::vec3 viscoForce = viscosity * mass * (velocityDif / pj->density) 
                           * spikyLap * (h - dist);
    
    return viscoForce;
// FUNCTION_ID: sph_func012 - END
}

void accumulateForcesFromNeighbor(Particle* pi, Particle* pj, float mass, float viscosity, float spikyGrad, float spikyLap, float h, glm::vec3& force) {
// FUNCTION_ID: sph_func013 - START
    float dist2 = glm::length2(pj->position - pi->position);
    if (dist2 >= h * h) {
        return;
    }
    
    float dist = std::sqrt(dist2);
    glm::vec3 dir = glm::normalize(pj->position - pi->position);
    
    // Pressure force
    glm::vec3 pressureForce = -dir * mass * (pi->pressure + pj->pressure) 
                              / (2 * pj->density) * spikyGrad;
    pressureForce *= std::pow(h - dist, 2);
    force += pressureForce;
    
    // Viscosity force
    glm::vec3 velocityDif = pj->velocity - pi->velocity;
    glm::vec3 viscoForce = viscosity * mass * (velocityDif / pj->density) 
                           * spikyLap * (h - dist);
    force += viscoForce;
// FUNCTION_ID: sph_func013 - END
}

// ============================================================================
// INTEGRATION FUNCTIONS
// ============================================================================

void integrateVelocity(Particle* p, float deltaTime, float g) {
// FUNCTION_ID: sph_func014 - START
    glm::vec3 acceleration = p->force / p->density + glm::vec3(0, g, 0);
    p->velocity += acceleration * deltaTime;
// FUNCTION_ID: sph_func014 - END
}

void integratePosition(Particle* p, float deltaTime) {
// FUNCTION_ID: sph_func015 - START
    p->position += p->velocity * deltaTime;
// FUNCTION_ID: sph_func015 - END
}

void handleGroundCollision(Particle* p, float h, float elasticity) {
// FUNCTION_ID: sph_func016 - START
    if (p->position.y < h) {
        p->position.y = -p->position.y + 2 * h + 0.0001f;
        p->velocity.y = -p->velocity.y * elasticity;
    }
// FUNCTION_ID: sph_func016 - END
}

void handleWallCollisionX(Particle* p, float h, float boxWidth, float elasticity) {
// FUNCTION_ID: sph_func017 - START
    if (p->position.x < h - boxWidth) {
        p->position.x = -p->position.x + 2 * (h - boxWidth) + 0.0001f;
        p->velocity.x = -p->velocity.x * elasticity;
    }
    
    if (p->position.x > -h + boxWidth) {
        p->position.x = -p->position.x + 2 * -(h - boxWidth) - 0.0001f;
        p->velocity.x = -p->velocity.x * elasticity;
    }
// FUNCTION_ID: sph_func017 - END
}

void handleWallCollisionZ(Particle* p, float h, float boxWidth, float elasticity) {
// FUNCTION_ID: sph_func018 - START
    if (p->position.z < h - boxWidth) {
        p->position.z = -p->position.z + 2 * (h - boxWidth) + 0.0001f;
        p->velocity.z = -p->velocity.z * elasticity;
    }
    
    if (p->position.z > -h + boxWidth) {
        p->position.z = -p->position.z + 2 * -(h - boxWidth) - 0.0001f;
        p->velocity.z = -p->velocity.z * elasticity;
    }
// FUNCTION_ID: sph_func018 - END
}

void applyBoundaryConditions(Particle* p, float h, float boxWidth, float elasticity) {
// FUNCTION_ID: sph_func019 - START
    // Ground collision
    if (p->position.y < h) {
        p->position.y = -p->position.y + 2 * h + 0.0001f;
        p->velocity.y = -p->velocity.y * elasticity;
    }
    
    // X-axis walls
    if (p->position.x < h - boxWidth) {
        p->position.x = -p->position.x + 2 * (h - boxWidth) + 0.0001f;
        p->velocity.x = -p->velocity.x * elasticity;
    }
    if (p->position.x > -h + boxWidth) {
        p->position.x = -p->position.x + 2 * -(h - boxWidth) - 0.0001f;
        p->velocity.x = -p->velocity.x * elasticity;
    }
    
    // Z-axis walls
    if (p->position.z < h - boxWidth) {
        p->position.z = -p->position.z + 2 * (h - boxWidth) + 0.0001f;
        p->velocity.z = -p->velocity.z * elasticity;
    }
    if (p->position.z > -h + boxWidth) {
        p->position.z = -p->position.z + 2 * -(h - boxWidth) - 0.0001f;
        p->velocity.z = -p->velocity.z * elasticity;
    }
// FUNCTION_ID: sph_func019 - END
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool isWithinSmoothingRadius(glm::vec3 pos1, glm::vec3 pos2, float h2) {
// FUNCTION_ID: sph_func020 - START
    float dist2 = glm::length2(pos2 - pos1);
    return dist2 < h2;
// FUNCTION_ID: sph_func020 - END
}

float calculateDistanceSquared(glm::vec3 p1, glm::vec3 p2) {
// FUNCTION_ID: sph_func021 - START
    return glm::length2(p2 - p1);
// FUNCTION_ID: sph_func021 - END
}

glm::vec3 normalizeDirection(glm::vec3 vec) {
// FUNCTION_ID: sph_func022 - START
    float len = glm::length(vec);
    if (len > 0.0001f) {
        return vec / len;
    }
    return glm::vec3(0, 0, 0);
// FUNCTION_ID: sph_func022 - END
}

glm::vec3 computeAcceleration(glm::vec3 force, float density, float g) {
// FUNCTION_ID: sph_func023 - START
    return force / density + glm::vec3(0, g, 0);
// FUNCTION_ID: sph_func023 - END
}

// ============================================================================
// PARALLEL COMPUTATION FUNCTIONS
// ============================================================================

void parallelCalculateHashes(Particle *particles, size_t start, size_t end, float h) {
// FUNCTION_ID: sph_func024 - START
    for (size_t i = start; i < end; i++) {
        Particle *particle = &particles[i];
        particle->hash = getHash(getCell(particle, h));
    }
// FUNCTION_ID: sph_func024 - END
}

void parallelDensityAndPressures(Particle *particles, const size_t particleCount, const size_t start, const size_t end, const uint32_t *particleTable, float mass, float poly6, float h, float h2, float selfDens, float restDensity, float gasConstant) {
// FUNCTION_ID: sph_func025 - START
    
    float massPoly6Product = mass * poly6;
    
    for (size_t piIndex = start; piIndex < end; piIndex++) {
        float pDensity = 0;
        Particle* pi = &particles[piIndex];
        glm::ivec3 cell = {pi->position.x / h, pi->position.y / h, pi->position.z / h};
        
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    glm::ivec3 neighborCell = cell + glm::ivec3(x, y, z);
                    uint16_t cellHash = ((uint)(neighborCell.x * 73856093) ^ (uint)(neighborCell.y * 19349663) ^ (uint)(neighborCell.z * 83492791)) % TABLE_SIZE;
                    uint32_t pjIndex = particleTable[cellHash];
                    if (pjIndex == NO_PARTICLE) {
                        continue;
                    }
                    while (pjIndex < particleCount) {
                        if (pjIndex == piIndex) {
                            pjIndex++;
                            continue;
                        }
                        Particle *pj = &particles[pjIndex];
                        if (pj->hash != cellHash) {
                            break;
                        }
                        float dist2 = glm::length2(pj->position - pi->position);
                        if (dist2 < h2) {
                            pDensity += massPoly6Product * glm::pow(h2 - dist2, 3);
                        }
                        pjIndex++;
                    }
                }
            }
        }
        
        // Include self density
        pi->density = pDensity + selfDens;
        
        // Calculate pressure
        float pPressure = gasConstant * (pi->density - restDensity);
        pi->pressure = pPressure;
    }
// FUNCTION_ID: sph_func025 - END
}

void parallelForces(Particle *particles, const size_t particleCount, const size_t start, const size_t end, const uint32_t *particleTable, float mass, float viscosity, float spikyGrad, float spikyLap, float h, float h2) {
// FUNCTION_ID: sph_func026 - START
    
    for (size_t piIndex = start; piIndex < end; piIndex++) {
        Particle* pi = &particles[piIndex];
        pi->force = glm::vec3(0);
        glm::ivec3 cell = {pi->position.x / h, pi->position.y / h, pi->position.z / h};
        
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    glm::ivec3 neighborCell = cell + glm::ivec3(x, y, z);
                    uint16_t cellHash = ((uint)(neighborCell.x * 73856093) ^ (uint)(neighborCell.y * 19349663) ^ (uint)(neighborCell.z * 83492791)) % TABLE_SIZE;
                    uint32_t pjIndex = particleTable[cellHash];
                    if (pjIndex == NO_PARTICLE) {
                        continue;
                    }
                    while (pjIndex < particleCount) {
                        if (pjIndex == piIndex) {
                            pjIndex++;
                            continue;
                        }
                        Particle *pj = &particles[pjIndex];
                        if (pj->hash != cellHash) {
                            break;
                        }
                        float dist2 = glm::length2(pj->position - pi->position);
                        if (dist2 < h2) {
                            float dist = std::sqrt(dist2);
                            glm::vec3 dir = glm::normalize(pj->position - pi->position);
                            
                            // Pressure force
                            glm::vec3 pressureForce = -dir * mass * (pi->pressure + pj->pressure) 
                                                      / (2 * pj->density) * spikyGrad;
                            pressureForce *= std::pow(h - dist, 2);
                            pi->force += pressureForce;
                            
                            // Viscosity force
                            glm::vec3 velocityDif = pj->velocity - pi->velocity;
                            glm::vec3 viscoForce = viscosity * mass * (velocityDif / pj->density) 
                                                   * spikyLap * (h - dist);
                            pi->force += viscoForce;
                        }
                        pjIndex++;
                    }
                }
            }
        }
    }
// FUNCTION_ID: sph_func026 - END
}

void parallelUpdateParticlePositions(Particle *particles, const size_t particleCount, const size_t start, const size_t end, float h, float boxWidth, float elasticity, float g, float deltaTime) {
// FUNCTION_ID: sph_func027 - START
    
    for (size_t i = start; i < end; i++) {
        Particle *p = &particles[i];
        
        // Calculate acceleration and velocity
        glm::vec3 acceleration = p->force / p->density + glm::vec3(0, g, 0);
        p->velocity += acceleration * deltaTime;
        
        // Update position
        p->position += p->velocity * deltaTime;
        
        // Handle collisions with box
        if (p->position.y < h) {
            p->position.y = -p->position.y + 2 * h + 0.0001f;
            p->velocity.y = -p->velocity.y * elasticity;
        }
        
        if (p->position.x < h - boxWidth) {
            p->position.x = -p->position.x + 2 * (h - boxWidth) + 0.0001f;
            p->velocity.x = -p->velocity.x * elasticity;
        }
        
        if (p->position.x > -h + boxWidth) {
            p->position.x = -p->position.x + 2 * -(h - boxWidth) - 0.0001f;
            p->velocity.x = -p->velocity.x * elasticity;
        }
        
        if (p->position.z < h - boxWidth) {
            p->position.z = -p->position.z + 2 * (h - boxWidth) + 0.0001f;
            p->velocity.z = -p->velocity.z * elasticity;
        }
        
        if (p->position.z > -h + boxWidth) {
            p->position.z = -p->position.z + 2 * -(h - boxWidth) - 0.0001f;
            p->velocity.z = -p->velocity.z * elasticity;
        }
    }
// FUNCTION_ID: sph_func027 - END
}
