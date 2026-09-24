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
    void MapBlockTextures();
    void CheckPlayerNewChunk();

    Player& player;
    std::unordered_map<unsigned long long, Chunk*> chunks; // keyed by ChunkKey(grid_x, grid_z)

    Texture2D texture_atlas;
    Material chunk_material;
    std::unordered_map<Block, BlockTexture> block_textures;

    int seed;
    int next_chunk_id = 0;



    // Vectors for constructing chunk mesh
    std::vector<float> v; // positions: 3 per vertex
    std::vector<float> n; // normals:   3 per vertex
    std::vector<float> t; // texcoords: 2 per vertex

    void PushFace(const float p[4][3], const float nrm[3], float tx, float ty, float tw, float th);
    static float* ToRL(const std::vector<float>& vec);
    void RebuildMesh(Chunk* c);
    void BuildChunkMesh(Chunk* c);
    bool Solid(Chunk* c, int x, int y, int z);
    bool SolidAtWorld(int wx, int wy, int wz);
};

#endif // !WORLD_H
