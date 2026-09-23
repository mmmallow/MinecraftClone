#ifndef COMMON_HPP
#define COMMON_HPP

#include <raylib.h>

constexpr Vector3 BLOCK_SIZE = { 1.0f, 1.0f, 1.0f };

enum Block {
    GRASS,
    DIRT,
    AIR,
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

/*struct Block {
    Vector3 position = {0.0f, 0.0f, 0.0f};
    BlockType type = AIR;
    bool is_solid = false;
};*/




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

#endif // !COMMON_H
