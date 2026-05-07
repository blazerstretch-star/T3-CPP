#include <mutex>

#include <neighborTable.h>

uint32_t getHash(const glm::ivec3 &cell)
{
    // FUNCTION_ID: sph_func001 - START
    return (
        (uint)(cell.x * 73856093)
        ^ (uint)(cell.y * 19349663)
        ^ (uint)(cell.z * 83492791)
    ) % TABLE_SIZE;
    // FUNCTION_ID: sph_func001 - END
}

glm::ivec3 getCell(Particle *p, float h)
{
    // FUNCTION_ID: sph_func002 - START
    return {p->position.x / h, p->position.y / h, p->position.z / h};
    // FUNCTION_ID: sph_func002 - END
}

uint32_t* createNeighborTable(
    Particle *sortedParticles, const size_t &particleCount)
{
    // FUNCTION_ID: sph_func003 - START
    uint32_t *particleTable
        = (uint32_t *)malloc(sizeof(uint32_t) * TABLE_SIZE);
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
