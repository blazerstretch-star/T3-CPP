#include "sph_types.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SPHSettings::SPHSettings(
    float mass, float restDensity, float gasConst, float viscosity,
    float h, float g, float tension)
    : mass(mass), restDensity(restDensity), gasConstant(gasConst),
      viscosity(viscosity), h(h), g(g), tension(tension)
{
    // Calculate kernel constants
    poly6 = 315.0f / (64.0f * M_PI * std::pow(h, 9));
    spikyGrad = -45.0f / (M_PI * std::pow(h, 6));
    spikyLap = 45.0f / (M_PI * std::pow(h, 6));
    
    // Calculate derived values
    h2 = h * h;
    selfDens = mass * poly6 * std::pow(h2, 3);
    massPoly6Product = mass * poly6;
    
    // Sphere scale for rendering
    sphereScale = glm::scale(glm::vec3(h / 2.0f));
}
