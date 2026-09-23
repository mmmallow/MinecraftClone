#ifndef CHUNK_H
#define CHUNK_H

#include "common.hpp"
#include <raylib.h>

class Chunk {
public:
    Chunk(int id, Vector3 pos);
    ~Chunk();
    void Destroy();

    Block& Get(Vector3 local);
    Block& GetFromWorld(Vector3 world);
    void Set(Vector3 local, Block block);
    void SetFromWorld(Vector3 world, Block block);

    Vector3 ToLocal(Vector3 world) const;

    int id;

    const int EDGE_LEN = 16; // # blocks on x and z axes
    const int Y_LEN    = 128; // # blocks on y-axis
    const int VOX_AREA = 16 * 16;
    const int VOX_VOL  = 16 * 128 * 16; // x-16, y-128, z-16
    Vector3 position;

    Block* data;
};

#endif // !CHUNK_H
