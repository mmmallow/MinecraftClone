#ifndef WORLD_H
#define WORLD_H

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <unordered_map>
#include "Player.h"
#include "Chunk.h"
#include "common.hpp"
#include "PerlinNoise.hpp"

class World {
public:
    World(Player& player, int seed = 0);
    ~World();
    void Destroy();

    void Update();
    void Draw();

private:
    void Generate();

    void CheckCollisions(Vector3 block);
    void DrawCubeTextureRec(BlockTexture block_tex, Vector3 position, float width, float height, float length, Color color);
    void MapBlockTextures();

    Player& player;
    std::unordered_map<int, Chunk*> chunks;

    Texture2D texture_atlas;
    std::unordered_map<Block, BlockTexture> block_textures;

    int seed;
    int next_chunk_id = 0;
};

#endif // !WORLD_H
