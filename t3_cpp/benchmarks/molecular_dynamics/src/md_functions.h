#ifndef MD_FUNCTIONS_H
#define MD_FUNCTIONS_H

#include "md_types.h"
#include <vector>

// Lennard-Jones potential and force calculations
double calculateLJPotential(const vec3& r, double epsilon, double sigma);
vec3 calculateLJForce(const vec3& r, double epsilon, double sigma);

// Periodic boundary conditions
vec3 applyPeriodicBoundary(const vec3& pos, double boxSize);
vec3 minimumImageDistance(const vec3& r1, const vec3& r2, double boxSize);

// Velocity Verlet integration
void velocityVerletStep1(vec3& pos, vec3& vel, const vec3& force, double mass, double dt);
void velocityVerletStep2(vec3& vel, const vec3& force, double mass, double dt);

// Energy calculations
double calculateKineticEnergy(const std::vector<vec3>& velocities, const std::vector<double>& masses);
double calculatePotentialEnergy(const std::vector<vec3>& positions, double boxSize, double cutoffRadius, double epsilon, double sigma);

// Temperature calculation
double calculateTemperature(const std::vector<vec3>& velocities, const std::vector<double>& masses);

// Anderson thermostat
void applyAndersonThermostat(std::vector<vec3>& velocities, const std::vector<double>& masses, double targetTemp, double collisionFreq, double dt);

// Initialization functions
void initializePositionsLattice(std::vector<vec3>& positions, double boxSize);
void initializeVelocitiesMB(std::vector<vec3>& velocities, const std::vector<double>& masses, double temperature);

// Force computation
void computeForces(const std::vector<vec3>& positions, std::vector<vec3>& forces, double boxSize, double cutoffRadius, double epsilon, double sigma);

#endif // MD_FUNCTIONS_H
