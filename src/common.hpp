#ifndef COMMON_HPP
#define COMMON_HPP

#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <cstdlib>
#include <string>

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
    LOG,
    LEAF,

    NONE,
};

static inline Block& increase_block (Block& block) {
    switch (block) {
        case GRASS: return block = DIRT;
        case DIRT: return block = STONE;
        case AIR: return block = STONE;
        case STONE: return block = SAND;
        case SAND: return block = LOG;
        case LOG: return block = LEAF;
        case LEAF: return block = GRASS;
        default: return block = GRASS;
    }
}

static inline Block& decrease_block (Block& block) {
    switch (block) {
        case GRASS: return block = LEAF;
        case DIRT: return block = GRASS;
        case AIR: return block = DIRT;
        case STONE: return block = DIRT;
        case SAND: return block = STONE;
        case LOG: return block = SAND;
        case LEAF: return block = LOG;
        default: return block = GRASS;
    }
}

struct BlockTexture {
    Vector2 size = {32.0f, 32.0f};
    Vector2 front;
    Vector2 back;
    Vector2 top;
    Vector2 bottom;
    Vector2 right;
    Vector2 left;
};

static inline std::string GetBlockName (Block block) {
    std::string b = "";
    switch (block) {
        case AIR:
        b = "Air";
        break;

        case GRASS:
        b = "Grass";
        break;

        case DIRT:
        b = "Dirt";
        break;

        case STONE:
        b = "Stone";
        break;

        case SAND:
        b = "Sand";
        break;

        case LOG:
        b = "Log";
        break;

        case LEAF:
        b = "Leaf";
        break;

        default:
        b = "None";
        break;
    }
    return b;
}

inline bool BlockIsSolid (Block block) {
    if (block == AIR) {
        return false;
    }

    return true;
}

inline bool BlockIsSeeThrough (Block block) {
    if (block == AIR)
        return true;
    else if (block == LEAF)
        return true;

    return false;
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

static inline float uniform_random (float min, float max) {
    return min + (rand() / (RAND_MAX + 1.0f)) * (max - min);
}

static inline Vector3 GetCameraRight(Camera* camera) {
    return Vector3Normalize(Vector3CrossProduct(camera->up, camera->target));
}

#endif // !COMMON_H
