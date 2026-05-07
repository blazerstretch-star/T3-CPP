#ifndef SPH_TYPES_H
#define SPH_TYPES_H

#include <glm/glm.hpp>
#include <cstdint>

// Particle structure
struct Particle {
    glm::vec3 position, velocity, acceleration, force;
    float density;
    float pressure;
    uint32_t hash;
};

// SPH Settings structure
struct SPHSettings {
    SPHSettings(
        float mass, float restDensity, float gasConst, float viscosity,
        float h, float g, float tension);

    glm::mat4 sphereScale;
    float poly6, spikyGrad, spikyLap, gasConstant, mass, h2, selfDens,
        restDensity, viscosity, h, g, tension, massPoly6Product;
};

// Constants
const uint32_t TABLE_SIZE = 262144;
const uint32_t NO_PARTICLE = 0xFFFFFFFF;

#endif // SPH_TYPES_H
