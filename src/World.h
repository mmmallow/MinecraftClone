#ifndef WORLD_H
#define WORLD_H

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "Player.h"
#include "Chunk.h"
#include "common.hpp"
#include "PerlinNoise.hpp"

class Player;

class World {
public:
    World(Player& player, int seed = 0);
    ~World();
    void Destroy();

    void Update();
    void Draw();

    bool Solid(Chunk* c, int x, int y, int z);
    bool SolidAtWorld(int wx, int wy, int wz);
    bool SeeThrough(Chunk* c, int x, int y, int z);
    bool SeeThroughAtWorld(int wx, int wy, int wz);
    Block BlockAt(Chunk* c, int x, int y, int z);
    std::pair<Chunk*, Block*> GetBlockAtWorld(int wx, int wy, int wz);
    void SetBlock(Vector3 pos, Block block);

private:
    void GenerateMap();
    void GenerateChunk(int grid_x, int grid_z);
    std::vector<Vector3> TreeGen(Ray ray);
    std::vector<Vector3> LeafGen(Vector3 center);
    std::string LSystem(std::string system, int count);

    void CheckCollisions(Vector3 block);
    void MapBlockTextures();
    bool CheckPlayerNewChunk();
    void LoadChunks();
    void UnloadChunks();

    int seed;
    int width = 512;
    int depth = 512;

    Player& player;
    std::unordered_map<unsigned long long, bool> chunks; // keyed by ChunkKey(grid_x, grid_z), value is if chunk has been generated
    std::unordered_map<unsigned long long, Chunk*> current_chunks;
    std::vector<std::vector<float>> map;

    Texture2D texture_atlas;
    Material chunk_material;
    Material water_material;
    std::unordered_map<Block, BlockTexture> block_textures;

    int water_time_loc = 0;


    // Vectors for constructing chunk mesh
    std::vector<float> v; // positions: 3 per vertex
    std::vector<float> n; // normals:   3 per vertex
    std::vector<float> t; // texcoords: 2 per vertex
    // Vectors for constructing chunk water mesh
    std::vector<float> wv; // positions: 3 per vertex
    std::vector<float> wn; // normals:   3 per vertex
    std::vector<float> wt; // texcoords: 2 per vertex

    void PushFace(const float p[4][3], const float nrm[3], float tx, float ty, float tw, float th, bool is_water);
    static float* ToRL(const std::vector<float>& vec);
    void RebuildMesh(Chunk* c);
    void BuildChunkMesh(Chunk* c);
};

#endif // !WORLD_H
