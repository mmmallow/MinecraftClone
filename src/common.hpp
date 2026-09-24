#ifndef COMMON_HPP
#define COMMON_HPP

#include <raylib.h>
#include <raymath.h>
#include <cmath>

constexpr Vector3 BLOCK_SIZE = { 1.0f, 1.0f, 1.0f };
static constexpr int CHUNK_EDGE_LEN = 16;  // # blocks on x and z axes
static constexpr int CHUNK_Y_LEN    = 128; // # blocks on y-axis
static constexpr int CHUNK_AREA = CHUNK_EDGE_LEN * CHUNK_EDGE_LEN;
static constexpr int CHUNK_VOL  = CHUNK_EDGE_LEN * CHUNK_Y_LEN * CHUNK_EDGE_LEN;

enum Block {
    GRASS,
    DIRT,
    AIR,
    STONE,
    SAND,
};

struct BlockTexture {
    Vector2 size = {32.0f, 32.0f};
    Vector2 front;
    Vector2 back;
    Vector2 top;
    Vector2 bottom;
    Vector2 right;
    Vector2 left;
};


inline bool BlockIsSolid (Block block) {
    if (block == AIR) {
        return false;
    }

    return true;
}

inline BoundingBox GetBoundingBox (Vector3 position, Vector3 size) {
    return (BoundingBox){(Vector3){ position.x - size.x/2,
                                    position.y - size.y/2,
                                    position.z - size.z/2 },
                         (Vector3){ position.x + size.x/2,
                                    position.y + size.y/2,
                                    position.z + size.z/2 }};
}


static inline int FloorDiv(int a, int b) {
    int q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) q--;
    return q;
}
static inline int PosMod(int a, int b) {
    int r = a % b;
    if (r < 0) r += b;
    return r;
}

static inline Vector3 GetCameraRight(Camera* camera) {
    return Vector3Normalize(Vector3CrossProduct(camera->up, camera->target));
}

#endif // !COMMON_H
