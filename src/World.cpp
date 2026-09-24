#include "World.h"
#include <rlgl.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cmath>

World::World (Player& player, int seed)
: player (player),
  seed (seed)
{
    texture_atlas = LoadTexture("assets/tex_atlas.png");
    MapBlockTextures();

    chunk_material = LoadMaterialDefault();
    SetMaterialTexture(&chunk_material, MATERIAL_MAP_DIFFUSE, texture_atlas);

    Generate();
    player.cur_chunk = chunks.at(ChunkKey(0, 0));
}

World::~World() {

}

void World::Destroy() {
    for (auto& pair : chunks) {
        if (pair.second->has_mesh) UnloadMesh(pair.second->mesh);
        pair.second->Destroy();
        delete pair.second;
    }

    RL_FREE(chunk_material.maps);
    UnloadTexture(texture_atlas);
}


void World::Update() {
    player.Update(GetFrameTime());
    CheckPlayerNewChunk();

    if (!player.is_freecam) {
        int n = 2; // radius
        int px = static_cast<int>(std::floor(player.position.x));
        int py = static_cast<int>(std::floor(player.position.y));
        int pz = static_cast<int>(std::floor(player.position.z));
        for (int y = -n; y < n+1; y++) {
            for (int z = -n; z < n+1; z++) {
                for (int x = -n; x < n+1; x++) {
                    int bx = px + x, by = py + y, bz = pz + z;
                    if (SolidAtWorld(bx, by, bz))
                        CheckCollisions({ static_cast<float>(bx),
                                          static_cast<float>(by),
                                          static_cast<float>(bz) });
                }
            }
        }
    }
}

void World::SetBlock (Vector3 pos, Block block) {
    auto pair = GetBlockAtWorld(pos.x, pos.y, pos.z);
    Chunk* chunk = pair.first;
    Block* old_block = pair.second;
    if (old_block != nullptr) {
        *old_block = block;
        chunk->dirty = true;
    }
}

void World::Draw() {
    bool draw_wires = IsKeyDown(KEY_P) ? true : false;
    ClearBackground(SKYBLUE);
    BeginMode3D(player.camera);
        if (draw_wires)
            rlEnableWireMode();
        else
            rlDisableWireMode();

        for (auto& pair : chunks) {
            Chunk* c = pair.second;
            if (c->dirty) RebuildMesh(c);
            if (!c->has_mesh) continue;
            DrawMesh(c->mesh, chunk_material, MatrixTranslate(c->position.x, c->position.y, c->position.z));
        }
        if (player.current_block.y <= 256.0f)
            DrawCubeWires(player.current_block, 1.0f, 1.0f, 1.0f, BLACK);
    EndMode3D();
}

void World::MapBlockTextures() {
    block_textures.insert({GRASS,{
                          {32.0f, 32.0f}, // size
                          {32.0f, 0.0f},  // front
                          {32.0f, 0.0f},  // back
                          {64.0f, 0.0f},  // top
                          {0.0f, 0.0f},   // bottom
                          {32.0f, 0.0f},  // right
                          {32.0f, 0.0f}   // left
    }});
    block_textures.insert({DIRT, {
                          {32.0f, 32.0f},
                          {0.0f, 0.0f},
                          {0.0f, 0.0f},
                          {0.0f, 0.0f},
                          {0.0f, 0.0f},
                          {0.0f, 0.0f},
                          {0.0f, 0.0f}
    }});
    block_textures.insert({STONE, {
                          {32.0f, 32.0f},
                          {96.0f, 0.0f},
                          {96.0f, 0.0f},
                          {96.0f, 0.0f},
                          {96.0f, 0.0f},
                          {96.0f, 0.0f},
                          {96.0f, 0.0f}
    }});
    block_textures.insert({SAND, {
                          {32.0f, 32.0f},
                          {128.0f, 0.0f},
                          {128.0f, 0.0f},
                          {128.0f, 0.0f},
                          {128.0f, 0.0f},
                          {128.0f, 0.0f},
                          {128.0f, 0.0f}
    }});
}


void World::CheckCollisions (Vector3 block) {
    BoundingBox player_box = GetBoundingBox(player.position, player.size);
    BoundingBox cube_box = GetBoundingBox(block, BLOCK_SIZE);

    if (!CheckCollisionBoxes(player_box, cube_box)) return;

    Vector3 push_neg = Vector3Subtract(player_box.max, cube_box.min); // move player -axis
    Vector3 push_pos = Vector3Subtract(cube_box.max, player_box.min); // move player +axis

    Vector3 depth = { fminf(push_neg.x, push_pos.x),
                      fminf(push_neg.y, push_pos.y),
                      fminf(push_neg.z, push_pos.z) };
    Vector3 sign = { (push_neg.x < push_pos.x) ? -1.0f : 1.0f,
                     (push_neg.y < push_pos.y) ? -1.0f : 1.0f,
                     (push_neg.z < push_pos.z) ? -1.0f : 1.0f };

    if (depth.y <= depth.x && depth.y <= depth.z) {
        player.position.y += sign.y*depth.y;
        player.velocity.y = 0.0f;
        if (sign.y > 0.0f) player.is_grounded = true; // landed on top of the block
    }
    else if (depth.x <= depth.y && depth.x <= depth.z) {
        player.position.x += sign.x*depth.x;
    }
    else {
        player.position.z += sign.z*depth.z;
    }
}


void World::CheckPlayerNewChunk() {
    int gx = FloorDiv(static_cast<int>(std::floor(player.position.x)), CHUNK_EDGE_LEN);
    int gz = FloorDiv(static_cast<int>(std::floor(player.position.z)), CHUNK_EDGE_LEN);

    if (player.cur_chunk != nullptr &&
        player.cur_chunk->grid_x == gx && player.cur_chunk->grid_z == gz)
        return;

    auto it = chunks.find(ChunkKey(gx, gz));
    if (it != chunks.end()) player.cur_chunk = it->second;
}

void World::Generate() {
    if (seed == 0) {
        std::srand(std::time(0));
        seed = std::rand();
    }
    int octaves = 5;
    PerlinNoise perlin { seed };

    // Generate height map
    int width = 16;
    int height = 16;
    float persistence = 0.5f;
    float lacunarity = 0.5f;
    float y_offset = 0;

    const float FREQUENCY = 0.02f; // noise units per block
    const int   BASE_Y    = 64;    // height a noise value of 0.5 maps to
    const int   AMPLITUDE = 32;    // how far terrain swings above/below BASE_Y

    int chunk_x = 0;
    int chunk_z = 0;


    for (chunk_z = 0; chunk_z < height; chunk_z++) {
        for (chunk_x = 0; chunk_x < width; chunk_x++) {
            Chunk* c = new Chunk(next_chunk_id, chunk_x, chunk_z);
            chunks.insert({ChunkKey(c->grid_x, c->grid_z), c});
            next_chunk_id++;

            std::vector<std::vector<float>> map(height, std::vector<float>(width));

            for (int z = 0; z < height; z++) {
                for (int x = 0; x < width; x++) {
                    float nx = (c->position.x + static_cast<float>(x)) * FREQUENCY;
                    float nz = (c->position.z + static_cast<float>(z)) * FREQUENCY;

                    map[z][x] = perlin.FractalNoise(nx, nz, y_offset,
                                                    octaves, persistence, lacunarity);
                }
            }

            for (int z = 0; z < CHUNK_EDGE_LEN; z++) {
                for (int x = 0; x < CHUNK_EDGE_LEN; x++) {
                    int surface_y = BASE_Y + static_cast<int>((map[z][x] - 0.5f) * 2.0f * AMPLITUDE);
                    Block type = GRASS;
                    if (surface_y < 20) {
                        type = STONE;
                    }
                    else if (surface_y < 60) {
                        type = SAND;
                    }
                    c->SetFromWorld({ c->position.x + static_cast<float>(x),
                                    static_cast<float>(surface_y),
                                    c->position.z + static_cast<float>(z) }, type);
                }
            }
        }
    }

}





void World::PushFace(const float p[4][3], const float nrm[3], float tx, float ty, float tw, float th) {
    const int tri[6] = {0,1,2, 0,2,3};
    const float uv[4][2] = {{tx, ty+th}, {tx+tw, ty+th}, {tx+tw, ty}, {tx, ty}};
    for (int i = 0; i < 6; i++) {
        int j = tri[i];
        v.insert(v.end(), {p[j][0], p[j][1], p[j][2]});
        n.insert(n.end(), {nrm[0], nrm[1], nrm[2]});
        t.insert(t.end(), {uv[j][0]/(float)texture_atlas.width,
                           uv[j][1]/(float)texture_atlas.height});
    }
}

float* World::ToRL(const std::vector<float>& vec) {
    float* p = (float*)RL_MALLOC(vec.size() * sizeof(float));
    memcpy(p, vec.data(), vec.size() * sizeof(float));
    return p;
}

void World::RebuildMesh (Chunk* c) {
    this->v.clear();
    this->n.clear();
    this->t.clear();
    BuildChunkMesh(c);
    if (c->has_mesh) {
        UnloadMesh(c->mesh);
        c->mesh = (Mesh){0};
        c->has_mesh = false;
    }
    if (this->v.empty()) {
        c->dirty = false;
        return;
    }

    c->mesh.vertexCount = (int)(this->v.size()/3);
    c->mesh.triangleCount = c->mesh.vertexCount / 3;
    c->mesh.vertices = ToRL(this->v);
    c->mesh.normals = ToRL(this->n);
    c->mesh.texcoords = ToRL(this->t);
    UploadMesh(&c->mesh, false);
    c->has_mesh = true;
    c->dirty = false;
}


bool World::Solid (Chunk* c, int x, int y, int z) {
    if (y < 0 || y >= CHUNK_Y_LEN) return false;

    if (x >= 0 && x < CHUNK_EDGE_LEN && z >= 0 && z < CHUNK_EDGE_LEN)
        return BlockIsSolid(c->data[x + z*CHUNK_EDGE_LEN + y*CHUNK_AREA]);

    return SolidAtWorld(c->grid_x * CHUNK_EDGE_LEN + x,
                        y,
                        c->grid_z * CHUNK_EDGE_LEN + z);
}

bool World::SolidAtWorld (int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_Y_LEN) return false;

    auto it = chunks.find(ChunkKey(FloorDiv(wx, CHUNK_EDGE_LEN),
                                   FloorDiv(wz, CHUNK_EDGE_LEN)));
    if (it == chunks.end()) return false;   // outside the generated world

    Chunk* c = it->second;
    int lx = PosMod(wx, CHUNK_EDGE_LEN);
    int lz = PosMod(wz, CHUNK_EDGE_LEN);
    return BlockIsSolid(c->data[lx + lz*CHUNK_EDGE_LEN + wy*CHUNK_AREA]);
}

std::pair<Chunk*, Block*> World::GetBlockAtWorld (int wx, int wy, int wz) {
    std::pair<Chunk*, Block*> result(nullptr, nullptr);
    if (wy < 0 || wy >= CHUNK_Y_LEN) return result;

    auto it = chunks.find(ChunkKey(FloorDiv(wx, CHUNK_EDGE_LEN),
                                   FloorDiv(wz, CHUNK_EDGE_LEN)));
    if (it == chunks.end()) return result;   // outside the generated world

    Chunk* c = it->second;
    int lx = PosMod(wx, CHUNK_EDGE_LEN);
    int lz = PosMod(wz, CHUNK_EDGE_LEN);
 
    result.first = c;
    result.second = &(c->data[lx + lz*CHUNK_EDGE_LEN + wy*CHUNK_AREA]);
    return result;
}

void World::BuildChunkMesh (Chunk* c) {
    int i = 0;
    for (int y = 0; y < CHUNK_Y_LEN; y++) {
        for (int z = 0; z < CHUNK_EDGE_LEN; z++) {
            for (int x = 0; x < CHUNK_EDGE_LEN; x++, i++) {
                Block block = c->data[i];
                if (!BlockIsSolid(block)) continue;

                BlockTexture bt = block_textures[block];
                const float cx = x, cy = y, cz = z, h = 0.5f;
                const float tw = bt.size.x, th = bt.size.y;

                if (!Solid(c, x, y, z + 1)) {   // front (+Z)
                    float p[4][3] = {{cx-h,cy-h,cz+h},{cx+h,cy-h,cz+h},{cx+h,cy+h,cz+h},{cx-h,cy+h,cz+h}};
                    float nrm[3] = {0.0f, 0.0f, 1.0f};
                    PushFace(p, nrm, bt.front.x, bt.front.y, tw, th);
                }
                if (!Solid(c, x, y, z - 1)) {   // back (-Z)
                    float p[4][3] = {{cx+h,cy-h,cz-h},{cx-h,cy-h,cz-h},{cx-h,cy+h,cz-h},{cx+h,cy+h,cz-h}};
                    float nrm[3] = {0.0f, 0.0f, -1.0f};
                    PushFace(p, nrm, bt.back.x, bt.back.y, tw, th);
                }
                if (!Solid(c, x, y + 1, z)) {   // top (+Y)
                    float p[4][3] = {{cx-h,cy+h,cz+h},{cx+h,cy+h,cz+h},{cx+h,cy+h,cz-h},{cx-h,cy+h,cz-h}};
                    float nrm[3] = {0.0f, 1.0f, 0.0f};
                    PushFace(p, nrm, bt.top.x, bt.top.y, tw, th);
                }
                if (!Solid(c, x, y - 1, z)) {   // bottom (-Y)
                    float p[4][3] = {{cx-h,cy-h,cz-h},{cx+h,cy-h,cz-h},{cx+h,cy-h,cz+h},{cx-h,cy-h,cz+h}};
                    float nrm[3] = {0.0f, -1.0f, 0.0f};
                    PushFace(p, nrm, bt.bottom.x, bt.bottom.y, tw, th);
                }
                if (!Solid(c, x + 1, y, z)) {   // right (+X)
                    float p[4][3] = {{cx+h,cy-h,cz+h},{cx+h,cy-h,cz-h},{cx+h,cy+h,cz-h},{cx+h,cy+h,cz+h}};
                    float nrm[3] = {1.0f, 0.0f, 0.0f};
                    PushFace(p, nrm, bt.right.x, bt.right.y, tw, th);
                }
                if (!Solid(c, x - 1, y, z)) {   // left (-X)
                    float p[4][3] = {{cx-h,cy-h,cz-h},{cx-h,cy-h,cz+h},{cx-h,cy+h,cz+h},{cx-h,cy+h,cz-h}};
                    float nrm[3] = {-1.0f, 0.0f, 0.0f};
                    PushFace(p, nrm, bt.left.x, bt.left.y, tw, th);
                }
            }
        }
    }
}
