#ifndef MDSIM_H
#define MDSIM_H

#include "vec.h"
#include <vector>
#include <cmath>

class MDSim {
private:
    std::vector<vec3> positions;
    std::vector<vec3> velocities;
    std::vector<vec3> forces;
    std::vector<double> masses;
    double dt;
    double boxSize;
    double cutoffRadius;
    double epsilon;
    double sigma;
    
public:
    MDSim(int numParticles, double timestep, double box) 
        : dt(timestep), boxSize(box), cutoffRadius(2.5), epsilon(1.0), sigma(1.0) {
        positions.resize(numParticles);
        velocities.resize(numParticles);
        forces.resize(numParticles);
        masses.resize(numParticles, 1.0);
    }
    
    // Lennard-Jones potential calculation
    double calculateLJPotential(const vec3& r, double epsilon, double sigma) {
        double dist = r.length();
        if (dist < 1e-10) return 0.0;
        
        double sigma_over_r = sigma / dist;
        double sigma_over_r6 = sigma_over_r * sigma_over_r * sigma_over_r * 
                                sigma_over_r * sigma_over_r * sigma_over_r;
        double sigma_over_r12 = sigma_over_r6 * sigma_over_r6;
        
        return 4.0 * epsilon * (sigma_over_r12 - sigma_over_r6);
    }
    
    // Lennard-Jones force calculation
    vec3 calculateLJForce(const vec3& r, double epsilon, double sigma) {
        double dist = r.length();
        if (dist < 1e-10) return vec3(0, 0, 0);
        
        double sigma_over_r = sigma / dist;
        double sigma_over_r6 = sigma_over_r * sigma_over_r * sigma_over_r * 
                                sigma_over_r * sigma_over_r * sigma_over_r;
        double sigma_over_r12 = sigma_over_r6 * sigma_over_r6;
        
        double forceMagnitude = 24.0 * epsilon * (2.0 * sigma_over_r12 - sigma_over_r6) / dist;
        return r * (forceMagnitude / dist);
    }
    
    // Apply periodic boundary conditions
    vec3 applyPeriodicBoundary(const vec3& pos, double boxSize) {
        vec3 result = pos;
        
        if (result.x < 0) result.x += boxSize;
        if (result.x >= boxSize) result.x -= boxSize;
        
        if (result.y < 0) result.y += boxSize;
        if (result.y >= boxSize) result.y -= boxSize;
        
        if (result.z < 0) result.z += boxSize;
        if (result.z >= boxSize) result.z -= boxSize;
        
        return result;
    }
    
    // Calculate minimum image distance
    vec3 minimumImageDistance(const vec3& r1, const vec3& r2, double boxSize) {
        vec3 dr = r2 - r1;
        
        if (dr.x > boxSize / 2.0) dr.x -= boxSize;
        if (dr.x < -boxSize / 2.0) dr.x += boxSize;
        
        if (dr.y > boxSize / 2.0) dr.y -= boxSize;
        if (dr.y < -boxSize / 2.0) dr.y += boxSize;
        
        if (dr.z > boxSize / 2.0) dr.z -= boxSize;
        if (dr.z < -boxSize / 2.0) dr.z += boxSize;
        
        return dr;
    }
    
    // Compute all pairwise forces
    void computeForces() {
        // Reset forces
        for (size_t i = 0; i < forces.size(); ++i) {
            forces[i] = vec3(0, 0, 0);
        }
        
        // Compute pairwise forces
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
    }
    
    // Velocity Verlet integration - first half
    void velocityVerletStep1(vec3& pos, vec3& vel, const vec3& force, double mass, double dt) {
        vel += force * (0.5 * dt / mass);
        pos += vel * dt;
    }
    
    // Velocity Verlet integration - second half
    void velocityVerletStep2(vec3& vel, const vec3& force, double mass, double dt) {
        vel += force * (0.5 * dt / mass);
    }
    
    // Calculate kinetic energy
    double calculateKineticEnergy(const std::vector<vec3>& velocities, 
                                   const std::vector<double>& masses) {
        double ke = 0.0;
        for (size_t i = 0; i < velocities.size(); ++i) {
            ke += 0.5 * masses[i] * velocities[i].lengthSquared();
        }
        return ke;
    }
    
    // Calculate potential energy
    double calculatePotentialEnergy() {
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
    }
    
    // Calculate temperature
    double calculateTemperature(const std::vector<vec3>& velocities, 
                                 const std::vector<double>& masses) {
        double ke = calculateKineticEnergy(velocities, masses);
        int degreesOfFreedom = 3 * velocities.size();
        double kB = 1.0; // Boltzmann constant in reduced units
        return (2.0 * ke) / (degreesOfFreedom * kB);
    }
    
    // Anderson thermostat
    void applyAndersonThermostat(std::vector<vec3>& velocities, 
                                  const std::vector<double>& masses,
                                  double targetTemp, double collisionFreq, double dt) {
        double kB = 1.0;
        
        for (size_t i = 0; i < velocities.size(); ++i) {
            double prob = collisionFreq * dt;
            double randVal = static_cast<double>(rand()) / RAND_MAX;
            
            if (randVal < prob) {
                // Reassign velocity from Maxwell-Boltzmann distribution
                double sigma_v = std::sqrt(kB * targetTemp / masses[i]);
                
                // Box-Muller transform for Gaussian random numbers
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
    }
    
    // Initialize positions on a lattice
    void initializePositionsLattice(std::vector<vec3>& positions, double boxSize) {
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
    }
    
    // Initialize velocities with Maxwell-Boltzmann distribution
    void initializeVelocitiesMB(std::vector<vec3>& velocities, 
                                 const std::vector<double>& masses, 
                                 double temperature) {
        double kB = 1.0;
        
        for (size_t i = 0; i < velocities.size(); ++i) {
            double sigma_v = std::sqrt(kB * temperature / masses[i]);
            
            // Box-Muller transform
            double u1 = static_cast<double>(rand()) / RAND_MAX;
            double u2 = static_cast<double>(rand()) / RAND_MAX;
            double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
            double z1 = std::sqrt(-2.0 * std::log(u1)) * std::sin(2.0 * M_PI * u2);
            
            u1 = static_cast<double>(rand()) / RAND_MAX;
            u2 = static_cast<double>(rand()) / RAND_MAX;
            double z2 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
            
            velocities[i] = vec3(z0 * sigma_v, z1 * sigma_v, z2 * sigma_v);
        }
        
        // Remove center of mass motion
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
    }
    
    // Full simulation step
    void step() {
        // Velocity Verlet step 1
        for (size_t i = 0; i < positions.size(); ++i) {
            velocityVerletStep1(positions[i], velocities[i], forces[i], masses[i], dt);
            positions[i] = applyPeriodicBoundary(positions[i], boxSize);
        }
        
        // Compute new forces
        computeForces();
        
        // Velocity Verlet step 2
        for (size_t i = 0; i < velocities.size(); ++i) {
            velocityVerletStep2(velocities[i], forces[i], masses[i], dt);
        }
    }
    
    // Getters
    const std::vector<vec3>& getPositions() const { return positions; }
    const std::vector<vec3>& getVelocities() const { return velocities; }
    const std::vector<vec3>& getForces() const { return forces; }
    const std::vector<double>& getMasses() const { return masses; }
    double getBoxSize() const { return boxSize; }
    double getTimestep() const { return dt; }
};

#endif // MDSIM_H
