#include "md_functions.h"
#include <cmath>
#include <cstdlib>
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


double calculateLJPotential(const vec3& r, double epsilon, double sigma) {
    // FUNCTION_ID: md_func001 - START
    double dist = r.length();
    if (dist < 1e-10) return 0.0;
    double sigma_over_r = sigma / dist;
    double sigma_over_r6 = sigma_over_r * sigma_over_r * sigma_over_r * sigma_over_r * sigma_over_r * sigma_over_r;
    double sigma_over_r12 = sigma_over_r6 * sigma_over_r6;
    return 4.0 * epsilon * (sigma_over_r12 - sigma_over_r6);
    // FUNCTION_ID: md_func001 - END
}

vec3 calculateLJForce(const vec3& r, double epsilon, double sigma) {
    // FUNCTION_ID: md_func002 - START
    double dist = r.length();
    if (dist < 1e-10) return vec3(0, 0, 0);
    double sigma_over_r = sigma / dist;
    double sigma_over_r6 = sigma_over_r * sigma_over_r * sigma_over_r * sigma_over_r * sigma_over_r * sigma_over_r;
    double sigma_over_r12 = sigma_over_r6 * sigma_over_r6;
    double forceMagnitude = 24.0 * epsilon * (2.0 * sigma_over_r12 - sigma_over_r6) / dist;
    return r * (forceMagnitude / dist);
    // FUNCTION_ID: md_func002 - END
}

vec3 applyPeriodicBoundary(const vec3& pos, double boxSize) {
    // FUNCTION_ID: md_func003 - START
    vec3 result = pos;
    if (result.x < 0) result.x += boxSize;
    if (result.x >= boxSize) result.x -= boxSize;
    if (result.y < 0) result.y += boxSize;
    if (result.y >= boxSize) result.y -= boxSize;
    if (result.z < 0) result.z += boxSize;
    if (result.z >= boxSize) result.z -= boxSize;
    return result;
    // FUNCTION_ID: md_func003 - END
}

vec3 minimumImageDistance(const vec3& r1, const vec3& r2, double boxSize) {
    // FUNCTION_ID: md_func004 - START
    vec3 dr = r2 - r1;
    if (dr.x > boxSize / 2.0) dr.x -= boxSize;
    if (dr.x < -boxSize / 2.0) dr.x += boxSize;
    if (dr.y > boxSize / 2.0) dr.y -= boxSize;
    if (dr.y < -boxSize / 2.0) dr.y += boxSize;
    if (dr.z > boxSize / 2.0) dr.z -= boxSize;
    if (dr.z < -boxSize / 2.0) dr.z += boxSize;
    return dr;
    // FUNCTION_ID: md_func004 - END
}

void velocityVerletStep1(vec3& pos, vec3& vel, const vec3& force, double mass, double dt) {
    // FUNCTION_ID: md_func005 - START
    vel += force * (0.5 * dt / mass);
    pos += vel * dt;
    // FUNCTION_ID: md_func005 - END
}

void velocityVerletStep2(vec3& vel, const vec3& force, double mass, double dt) {
    // FUNCTION_ID: md_func006 - START
    vel += force * (0.5 * dt / mass);
    // FUNCTION_ID: md_func006 - END
}

double calculateKineticEnergy(const std::vector<vec3>& velocities, const std::vector<double>& masses) {
    // FUNCTION_ID: md_func007 - START
    double ke = 0.0;
    for (size_t i = 0; i < velocities.size(); ++i) {
        ke += 0.5 * masses[i] * velocities[i].lengthSquared();
    }
    return ke;
    // FUNCTION_ID: md_func007 - END
}

double calculatePotentialEnergy(const std::vector<vec3>& positions, double boxSize, double cutoffRadius, double epsilon, double sigma) {
    // FUNCTION_ID: md_func008 - START
    double pe = 0.0;
    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            vec3 dr = minimumImageDistance(positions[i], positions[j], boxSize);
            double dist = dr.length();
            if (dist < cutoffRadius) {
                pe += calculateLJPotential(dr, epsilon, sigma);
            }
        }
    }
    return pe;
    // FUNCTION_ID: md_func008 - END
}

double calculateTemperature(const std::vector<vec3>& velocities, const std::vector<double>& masses) {
    // FUNCTION_ID: md_func009 - START
    double ke = calculateKineticEnergy(velocities, masses);
    int degreesOfFreedom = 3 * velocities.size();
    double kB = 1.0;
    return (2.0 * ke) / (degreesOfFreedom * kB);
    // FUNCTION_ID: md_func009 - END
}

void applyAndersonThermostat(std::vector<vec3>& velocities, const std::vector<double>& masses, double targetTemp, double collisionFreq, double dt) {
    // FUNCTION_ID: md_func010 - START
    double kB = 1.0;
    for (size_t i = 0; i < velocities.size(); ++i) {
        double prob = collisionFreq * dt;
        double randVal = static_cast<double>(rand()) / RAND_MAX;
        if (randVal < prob) {
            double sigma_v = std::sqrt(kB * targetTemp / masses[i]);
            double u1 = static_cast<double>(rand()) / RAND_MAX;
            double u2 = static_cast<double>(rand()) / RAND_MAX;
            double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
            double z1 = std::sqrt(-2.0 * std::log(u1)) * std::sin(2.0 * M_PI * u2);
            u1 = static_cast<double>(rand()) / RAND_MAX;
            u2 = static_cast<double>(rand()) / RAND_MAX;
            double z2 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
            velocities[i] = vec3(z0 * sigma_v, z1 * sigma_v, z2 * sigma_v);
        }
    }
    // FUNCTION_ID: md_func010 - END
}

void initializePositionsLattice(std::vector<vec3>& positions, double boxSize) {
    // FUNCTION_ID: md_func011 - START
    int n = positions.size();
    int nCube = static_cast<int>(std::ceil(std::pow(n, 1.0/3.0)));
    double spacing = boxSize / nCube;
    int idx = 0;
    for (int i = 0; i < nCube && idx < n; ++i) {
        for (int j = 0; j < nCube && idx < n; ++j) {
            for (int k = 0; k < nCube && idx < n; ++k) {
                positions[idx] = vec3(i * spacing, j * spacing, k * spacing);
                ++idx;
            }
        }
    }
    // FUNCTION_ID: md_func011 - END
}

void initializeVelocitiesMB(std::vector<vec3>& velocities, const std::vector<double>& masses, double temperature) {
    // FUNCTION_ID: md_func012 - START
    double kB = 1.0;
    for (size_t i = 0; i < velocities.size(); ++i) {
        double sigma_v = std::sqrt(kB * temperature / masses[i]);
        double u1 = static_cast<double>(rand()) / RAND_MAX;
        double u2 = static_cast<double>(rand()) / RAND_MAX;
        double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
        double z1 = std::sqrt(-2.0 * std::log(u1)) * std::sin(2.0 * M_PI * u2);
        u1 = static_cast<double>(rand()) / RAND_MAX;
        u2 = static_cast<double>(rand()) / RAND_MAX;
        double z2 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
        velocities[i] = vec3(z0 * sigma_v, z1 * sigma_v, z2 * sigma_v);
    }
    vec3 vcm(0, 0, 0);
    double totalMass = 0.0;
    for (size_t i = 0; i < velocities.size(); ++i) {
        vcm += velocities[i] * masses[i];
        totalMass += masses[i];
    }
    vcm /= totalMass;
    for (size_t i = 0; i < velocities.size(); ++i) {
        velocities[i] -= vcm;
    }
    // FUNCTION_ID: md_func012 - END
}

void computeForces(const std::vector<vec3>& positions, std::vector<vec3>& forces, double boxSize, double cutoffRadius, double epsilon, double sigma) {
    // FUNCTION_ID: md_func013 - START
    for (size_t i = 0; i < forces.size(); ++i) {
        forces[i] = vec3(0, 0, 0);
    }
    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            vec3 dr = minimumImageDistance(positions[i], positions[j], boxSize);
            double dist = dr.length();
            if (dist < cutoffRadius) {
                vec3 force = calculateLJForce(dr, epsilon, sigma);
                forces[i] += force;
                forces[j] -= force;
            }
        }
    }
    // FUNCTION_ID: md_func013 - END
}
