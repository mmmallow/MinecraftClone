#ifndef CHUNK_H
#define CHUNK_H

#include "common.hpp"
#include <raylib.h>

inline unsigned long long ChunkKey(int grid_x, int grid_z) {
    unsigned long long ux = static_cast<unsigned int>(grid_x);
    unsigned long long uz = static_cast<unsigned int>(grid_z);
    return (ux << 32) | uz;
}

class Chunk {
public:
    Chunk(int grid_x, int grid_z);
    ~Chunk();
    void Destroy();

    Block& Get(Vector3 local);
    Block& GetFromWorld(Vector3 world);
    void Set(Vector3 local, Block block);
    void SetFromWorld(Vector3 world, Block block);

    Vector3 ToLocal(Vector3 world) const;

    int grid_x;
    int grid_z;
    Vector3 position;

    Block* data;

    Mesh mesh = { 0 };
    bool has_mesh = false;
    bool dirty = true;
};

#endif // !CHUNK_H
