#ifndef WORLD_H
#define WORLD_H

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <unordered_map>
#include "Player.h"
#include "common.hpp"

class World {
public:
    World(Player& player);
    ~World();
    void Destroy();

    void Update();
    void Draw();

private:
    void CheckCollisions(Block block);
    void DrawCubeTextureRec(BlockTexture block_tex, Vector3 position, float width, float height, float length, Color color);
    void MapBlockTextures();

    Player& player;
    std::vector<Block> blocks;

    Texture2D texture_atlas;
    std::unordered_map<BlockType, BlockTexture> block_textures;
};

#endif // !WORLD_H
