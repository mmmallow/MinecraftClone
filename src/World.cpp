#include "World.h"
#include <rlgl.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

World::World (Player& player, int seed)
: player (player),
  seed (seed),
  map (depth, std::vector<float>(width))
{
    texture_atlas = LoadTexture("assets/tex_atlas.png");
    MapBlockTextures();

    chunk_material = LoadMaterialDefault();
    water_material = LoadMaterialDefault();
    chunk_material.shader = LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
    water_material.shader = LoadShader("assets/shaders/water.vs", "assets/shaders/water.fs");
    water_time_loc = GetShaderLocation(water_material.shader, "time");
    SetMaterialTexture(&chunk_material, MATERIAL_MAP_DIFFUSE, texture_atlas);
    SetMaterialTexture(&water_material, MATERIAL_MAP_DIFFUSE, texture_atlas);

    GenerateMap();
    GenerateChunk(8, 8);
    player.cur_chunk = current_chunks.at(ChunkKey(8, 8));
    player.position = { 136.0f, 90.0f, 136.0f };
    LoadChunks();
}

World::~World() {

}

void World::Destroy() {
    for (auto& pair : current_chunks) {
        if (pair.second->has_mesh) UnloadMesh(pair.second->mesh);
        if (pair.second->has_water_mesh) UnloadMesh(pair.second->water_mesh);
        pair.second->Destroy();
        delete pair.second;
    }

    UnloadShader(chunk_material.shader);
    UnloadShader(water_material.shader);
    RL_FREE(chunk_material.maps);
    RL_FREE(water_material.maps);
    UnloadTexture(texture_atlas);
}


void World::Update() {
    player.Update(GetFrameTime());
    if (CheckPlayerNewChunk())
        LoadChunks();

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
        if (pos.x == chunk->position.x || pos.x == chunk->position.x + CHUNK_EDGE_LEN-1) {
            int sign = pos.x == chunk->position.x ? -1 : 1;
            auto neighbor_x = current_chunks.find(ChunkKey(chunk->grid_x+sign, chunk->grid_z));
            if (neighbor_x != current_chunks.end())
                neighbor_x->second->dirty = true;
        }
        if (pos.z == chunk->position.z || pos.z == chunk->position.z + CHUNK_EDGE_LEN-1) {
            int sign = pos.z == chunk->position.z ? -1 : 1;
            auto neighbor_z = current_chunks.find(ChunkKey(chunk->grid_x, chunk->grid_z+sign));
            if (neighbor_z != current_chunks.end())
                neighbor_z->second->dirty = true;
        }
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

        for (auto& pair : current_chunks) {
            Chunk* c = pair.second;
            if (c->dirty) RebuildMesh(c);
            if (c->has_mesh)
                DrawMesh(c->mesh, chunk_material, MatrixTranslate(c->position.x, c->position.y, c->position.z));
        }

        float t = (float)GetTime();
        SetShaderValue(water_material.shader, water_time_loc, &t, SHADER_UNIFORM_FLOAT);
        rlDisableDepthMask();
        for (auto& pair : current_chunks) {
            Chunk* c = pair.second;
            if (c->has_water_mesh)
                DrawMesh(c->water_mesh, water_material, MatrixTranslate(c->position.x, c->position.y, c->position.z));
        }
        rlEnableDepthMask();
        if (player.current_block_looking.position.x != 0.0f && player.current_block_looking.position.y != 256.0f && player.current_block_looking.position.z != 0.0f)
            DrawCubeWires(player.current_block_looking.position, 1.0f, 1.0f, 1.0f, BLACK);
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

    block_textures.insert({LOG,{
                          {32.0f, 32.0f},
                          {0.0f, 32.0f},
                          {0.0f, 32.0f},
                          {32.0f, 32.0f},
                          {32.0f, 32.0f},
                          {0.0f, 32.0f},
                          {0.0f, 32.0f}
    }});
    block_textures.insert({LEAF, {
                          {32.0f, 32.0f},
                          {64.0f, 32.0f},
                          {64.0f, 32.0f},
                          {64.0f, 32.0f},
                          {64.0f, 32.0f},
                          {64.0f, 32.0f},
                          {64.0f, 32.0f}
    }});
    block_textures.insert({GLASS, {
                          {32.0f, 32.0f},
                          {96.0f, 32.0f},
                          {96.0f, 32.0f},
                          {96.0f, 32.0f},
                          {96.0f, 32.0f},
                          {96.0f, 32.0f},
                          {96.0f, 32.0f}
    }});
    block_textures.insert({PLANK, {
                          {32.0f, 32.0f},
                          {128.0f, 32.0f},
                          {128.0f, 32.0f},
                          {128.0f, 32.0f},
                          {128.0f, 32.0f},
                          {128.0f, 32.0f},
                          {128.0f, 32.0f}
    }});

    block_textures.insert({WATER, {
                          {32.0f, 32.0f},
                          {0.0f, 64.0f},
                          {0.0f, 64.0f},
                          {0.0f, 64.0f},
                          {0.0f, 64.0f},
                          {0.0f, 64.0f},
                          {0.0f, 64.0f}
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


bool World::CheckPlayerNewChunk() {
    int gx = FloorDiv(static_cast<int>(std::floor(player.position.x)), CHUNK_EDGE_LEN);
    int gz = FloorDiv(static_cast<int>(std::floor(player.position.z)), CHUNK_EDGE_LEN);

    if (player.cur_chunk != nullptr &&
        player.cur_chunk->grid_x == gx && player.cur_chunk->grid_z == gz)
        return false;

    auto it = current_chunks.find(ChunkKey(gx, gz));
    if (it != current_chunks.end()) {
        player.cur_chunk = it->second;
        return true;
    }

    return false;
}

void World::LoadChunks() {
    UnloadChunks();

    Chunk* cur = player.cur_chunk;
    for (int z = cur->grid_z - player.chunk_radius; z < cur->grid_z + player.chunk_radius; z++) {
        for (int x = cur->grid_x - player.chunk_radius; x < cur->grid_x + player.chunk_radius; x++) {

            auto it = current_chunks.find(ChunkKey(x, z));

            if (it == current_chunks.end()) {
                auto map_it = chunks.find(ChunkKey(x, z));
                if (map_it != chunks.end()) {
                    if (map_it->second == true) {
                        std::string file_path = "data/" + std::to_string(x) + "-" + std::to_string(z) + ".dat";
                        current_chunks.insert({ChunkKey(x, z), new Chunk(x, z, file_path)});
                    }
                    else {
                        GenerateChunk(x, z);
                    }
                }
            }
        }
    }
}

void World::UnloadChunks() {
    Chunk* cur = player.cur_chunk;
    for (auto it = current_chunks.begin(); it != current_chunks.end(); ) {
        Chunk* c = it->second;
        if (c->grid_x > player.chunk_radius + cur->grid_x || c->grid_x < cur->grid_x - player.chunk_radius || c->grid_z > player.chunk_radius + cur->grid_z || c->grid_z < cur->grid_z - player.chunk_radius) {
            c->Serialize("data/" + std::to_string(c->grid_x) + "-" + std::to_string(c->grid_z) + ".dat");

            if (c->has_mesh) UnloadMesh(c->mesh);
            if (c->has_water_mesh) UnloadMesh(c->water_mesh);
            c->Destroy();
            delete c;

            it = current_chunks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void World::GenerateMap() {
    if (seed == 0) {
        std::srand(std::time(0));
        seed = std::rand();
    }
    //seed = 777;
    int octaves = 5;
    PerlinNoise perlin { seed };
    srand(seed);

    float persistence = 0.5f;
    float lacunarity = 0.5f;
    float y_offset = 0;

    const float FREQUENCY = 0.03f; // noise units per block

    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            float nx = static_cast<float>(x) * FREQUENCY;
            float nz = static_cast<float>(z) * FREQUENCY;

            map[z][x] = perlin.FractalNoise(nx, nz, y_offset,
                                            octaves, persistence, lacunarity);

            if (z % CHUNK_EDGE_LEN == 0 && x % CHUNK_EDGE_LEN == 0)
                chunks.insert({ChunkKey(z / CHUNK_EDGE_LEN, x / CHUNK_EDGE_LEN), false});
        }
    }
}

void World::GenerateChunk (int grid_x, int grid_z) {
    const int   BASE_Y    = 64;    // height a noise value of 0.5 maps to
    const int   AMPLITUDE = 32;    // how far terrain swings above/below BASE_Y

    std::vector<Vector3> leaves;
    std::vector<Vector3> logs;

    Chunk* c = new Chunk(grid_x, grid_z);
    current_chunks.insert({ChunkKey(grid_x, grid_z), c});
    auto it = chunks.find(ChunkKey(grid_x, grid_z));
    if (it != chunks.end()) it->second = true;
    else return;

    int middle_y = BASE_Y;
    for (int z = grid_z * CHUNK_EDGE_LEN; z < grid_z * CHUNK_EDGE_LEN + CHUNK_EDGE_LEN; z++) {
        for (int x = grid_x * CHUNK_EDGE_LEN; x < grid_x * CHUNK_EDGE_LEN + CHUNK_EDGE_LEN; x++) {
            // Generate surface blocks
            int surface_y = BASE_Y + static_cast<int>((map[z][x] - 0.5f) * 2.0f * AMPLITUDE);
            Block type = GRASS;
            if (surface_y < 20) {
                type = STONE;
            }
            else if (surface_y < 60) {
                type = SAND;
            }
            c->SetFromWorld({ static_cast<float>(x),
                            static_cast<float>(surface_y),
                            static_cast<float>(z) }, type);

            // Generate trees
            if (type == GRASS) {
                int random_num = rand() % (900);
                if (random_num == 899) {
                    c->SetFromWorld({static_cast<float>(x),
                                    static_cast<float>(surface_y+1),
                                    static_cast<float>(z) }, LOG);
                    c->SetFromWorld({ static_cast<float>(x),
                                    static_cast<float>(surface_y+2),
                                    static_cast<float>(z) }, LOG);
                    c->SetFromWorld({ static_cast<float>(x),
                                    static_cast<float>(surface_y+3),
                                    static_cast<float>(z) }, LOG);

                    for (int i = 0; i < 3; i++) {
                        Vector3 direction = {uniform_random(-1.0f, 1.0f), 1.0f, uniform_random(-1.0f, 1.0f)};
                        Ray ray = { {static_cast<float>(x), static_cast<float>(surface_y+3), static_cast<float>(z)},
                                    Vector3Normalize(direction)};
                        auto vec = TreeGen(ray);
                        logs.insert(logs.end(), vec.begin(), vec.end());

                        auto leaf = LeafGen(logs.back());
                        leaves.insert(leaves.end(), leaf.begin(), leaf.end());
                    }


                }
            }

            if (type == SAND) {
                int water_y = surface_y+1;
                while (water_y <= 60) {
                    c->SetFromWorld({ static_cast<float>(x),
                                    static_cast<float>(water_y++),
                                    static_cast<float>(z) }, WATER);
                }
            }

            // Fill chunk with blocks
            for (int y = surface_y-1; y >= 0; y--) {
                if (type == SAND) {
                    c->SetFromWorld({ static_cast<float>(x),
                                    static_cast<float>(y--),
                                    static_cast<float>(z) }, type);
                    c->SetFromWorld({ static_cast<float>(x),
                                    static_cast<float>(y--),
                                    static_cast<float>(z) }, type);
                    c->SetFromWorld({ static_cast<float>(x),
                                    static_cast<float>(y--),
                                    static_cast<float>(z) }, type);
                    type = DIRT;
                }
                if (y < 22) {
                    type = STONE;

                }
                else type = DIRT;
                c->SetFromWorld({ static_cast<float>(x),
                                static_cast<float>(y),
                                static_cast<float>(z) }, type);
            }

            if (z == CHUNK_EDGE_LEN / 2 && x == CHUNK_EDGE_LEN / 2)
                middle_y = surface_y;
        }
    }

    for (auto& log : logs) {
        Block* block = GetBlockAtWorld(log.x, log.y, log.z).second;
        if (block != nullptr && *block == AIR)
            *block = LOG;
    }
    for (auto& leaf : leaves) {
        Block* block = GetBlockAtWorld(leaf.x, leaf.y, leaf.z).second;
        if (block != nullptr && *block == AIR)
            *block = LEAF;
    }

}

std::vector<Vector3> World::TreeGen (Ray ray) {
    std::vector<Vector3> result;
    std::string system = LSystem("T", 2);
    float angle = 4.0f;
    for (char& s : system) {
        if (s == 'T') {
            ray.position = Vector3Add(ray.position, ray.direction);
            result.push_back({ std::floor(ray.position.x), std::floor(ray.position.y), std::floor(ray.position.z) });
        }
        else if (s == '+') {
            Matrix rot = MatrixRotateY(angle * DEG2RAD);
            ray.direction = Vector3Transform(ray.direction, rot);
        }
        else if (s == '-') {
            Matrix rot = MatrixRotateY(-angle * DEG2RAD);
            ray.direction = Vector3Transform(ray.direction, rot);
        }
        else if (s == '>') {
            Matrix rot = MatrixRotateX(angle * DEG2RAD);
            ray.direction = Vector3Transform(ray.direction, rot);
        }
        else if (s == '<') {
            Matrix rot = MatrixRotateX(-angle * DEG2RAD);
            ray.direction = Vector3Transform(ray.direction, rot);
        }
    }

    return result;
}

std::vector<Vector3> World::LeafGen (Vector3 center) {
    std::vector<Vector3> leaves;
    leaves.push_back(center);
    int num_leaves = 110 + rand() % (145 - 110);
    for (int i = 0; i < num_leaves; i++) {
        Vector3 direction = { uniform_random(-1.0f, 1.0f), uniform_random(-1.0f, 1.0f), uniform_random(-1.0f, 1.0f) };
        float dist = uniform_random(0, 2.0f);
        direction = Vector3Scale(direction, dist);
        leaves.push_back(Vector3Add(center, direction));
    }

    return leaves;
}

std::string World::LSystem (std::string system, int count) {
    if (count == 0)
        return system;

    std::string result = "";
    for (char& s : system) {
        switch (s) {
            case 'T':
            result += "T+>";
            break;

            case '+':
            result += "+<T";
            break;

            case '-':
            result += "+T>";
            break;

            case '>':
            result += "<T+";
            break;

            case '<':
            result += ">-T";
            break;
        }
    }
 
    count--;
    return LSystem(result, count);
}


void World::PushFace(const float p[4][3], const float nrm[3], float tx, float ty, float tw, float th, bool is_water) {
    const int tri[6] = {0,1,2, 0,2,3};
    const float uv[4][2] = {{tx, ty+th}, {tx+tw, ty+th}, {tx+tw, ty}, {tx, ty}};
    for (int i = 0; i < 6; i++) {
        int j = tri[i];
        if (is_water) {
            wv.insert(wv.end(), {p[j][0], p[j][1], p[j][2]});
            wn.insert(wn.end(), {nrm[0], nrm[1], nrm[2]});
            wt.insert(wt.end(), {uv[j][0]/(float)texture_atlas.width,
                            uv[j][1]/(float)texture_atlas.height});
        }
        else {
            v.insert(v.end(), {p[j][0], p[j][1], p[j][2]});
            n.insert(n.end(), {nrm[0], nrm[1], nrm[2]});
            t.insert(t.end(), {uv[j][0]/(float)texture_atlas.width,
                            uv[j][1]/(float)texture_atlas.height});
        }
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
    this->wv.clear();
    this->wn.clear();
    this->wt.clear();
    BuildChunkMesh(c);
    if (c->has_mesh) {
        UnloadMesh(c->mesh);
        c->mesh = (Mesh){0};
        c->has_mesh = false;
    }
    if (c->has_water_mesh) {
        UnloadMesh(c->water_mesh);
        c->water_mesh = (Mesh){0};
        c->has_water_mesh = false;
    }

    if (!this->v.empty()) {
        c->mesh.vertexCount = (int)(this->v.size()/3);
        c->mesh.triangleCount = c->mesh.vertexCount / 3;
        c->mesh.vertices = ToRL(this->v);
        c->mesh.normals = ToRL(this->n);
        c->mesh.texcoords = ToRL(this->t);
        UploadMesh(&c->mesh, false);
        c->has_mesh = true;
    }
    if (!this->wv.empty()) {
        c->water_mesh.vertexCount = (int)(this->wv.size()/3);
        c->water_mesh.triangleCount = c->water_mesh.vertexCount / 3;
        c->water_mesh.vertices = ToRL(this->wv);
        c->water_mesh.normals = ToRL(this->wn);
        c->water_mesh.texcoords = ToRL(this->wt);
        UploadMesh(&c->water_mesh, false);
        c->has_water_mesh = true;
    }
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

    auto it = current_chunks.find(ChunkKey(FloorDiv(wx, CHUNK_EDGE_LEN),
                                   FloorDiv(wz, CHUNK_EDGE_LEN)));
    if (it == current_chunks.end()) return false;   // outside the generated world

    Chunk* c = it->second;
    if (c == nullptr)
        return false;
    int lx = PosMod(wx, CHUNK_EDGE_LEN);
    int lz = PosMod(wz, CHUNK_EDGE_LEN);
    return BlockIsSolid(c->data[lx + lz*CHUNK_EDGE_LEN + wy*CHUNK_AREA]);
}

bool World::SeeThrough (Chunk* c, int x, int y, int z) {
    if (y < 0 || y >= CHUNK_Y_LEN) return true;

    if (x >= 0 && x < CHUNK_EDGE_LEN && z >= 0 && z < CHUNK_EDGE_LEN)
        return BlockIsSeeThrough(c->data[x + z*CHUNK_EDGE_LEN + y*CHUNK_AREA]);

    return SeeThroughAtWorld(c->grid_x * CHUNK_EDGE_LEN + x,
                             y,
                             c->grid_z * CHUNK_EDGE_LEN + z);
}

bool World::SeeThroughAtWorld (int wx, int wy, int wz) {
    if (wy < 0 || wy >= CHUNK_Y_LEN) return true;

    auto it = current_chunks.find(ChunkKey(FloorDiv(wx, CHUNK_EDGE_LEN),
                                   FloorDiv(wz, CHUNK_EDGE_LEN)));
    if (it == current_chunks.end()) return true;   // outside the generated world

    Chunk* c = it->second;
    int lx = PosMod(wx, CHUNK_EDGE_LEN);
    int lz = PosMod(wz, CHUNK_EDGE_LEN);
    return BlockIsSeeThrough(c->data[lx + lz*CHUNK_EDGE_LEN + wy*CHUNK_AREA]);
}

// Block at chunk-local coords, looking into neighbouring chunks
Block World::BlockAt (Chunk* c, int x, int y, int z) {
    if (y < 0 || y >= CHUNK_Y_LEN) return AIR;

    if (x >= 0 && x < CHUNK_EDGE_LEN && z >= 0 && z < CHUNK_EDGE_LEN)
        return c->data[x + z*CHUNK_EDGE_LEN + y*CHUNK_AREA];

    auto result = GetBlockAtWorld(c->grid_x * CHUNK_EDGE_LEN + x,
                                  y,
                                  c->grid_z * CHUNK_EDGE_LEN + z);
    return result.second ? *result.second : AIR;
}

std::pair<Chunk*, Block*> World::GetBlockAtWorld (int wx, int wy, int wz) {
    std::pair<Chunk*, Block*> result(nullptr, nullptr);
    if (wy < 0 || wy >= CHUNK_Y_LEN) return result;

    auto it = current_chunks.find(ChunkKey(FloorDiv(wx, CHUNK_EDGE_LEN),
                                   FloorDiv(wz, CHUNK_EDGE_LEN)));
    if (it == current_chunks.end()) return result;   // outside the generated world

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
                if (block == AIR) continue;
                const bool is_water = (block == WATER);

                BlockTexture bt = block_textures[block];
                const float cx = x, cy = y, cz = z, h = 0.5f;
                const float tw = bt.size.x, th = bt.size.y;

                if (FaceVisible(block, BlockAt(c, x, y, z + 1))) {   // front (+Z)
                    float p[4][3] = {{cx-h,cy-h,cz+h},{cx+h,cy-h,cz+h},{cx+h,cy+h,cz+h},{cx-h,cy+h,cz+h}};
                    float nrm[3] = {0.0f, 0.0f, 1.0f};
                    PushFace(p, nrm, bt.front.x, bt.front.y, tw, th, is_water);
                }
                if (FaceVisible(block, BlockAt(c, x, y, z - 1))) {   // back (-Z)
                    float p[4][3] = {{cx+h,cy-h,cz-h},{cx-h,cy-h,cz-h},{cx-h,cy+h,cz-h},{cx+h,cy+h,cz-h}};
                    float nrm[3] = {0.0f, 0.0f, -1.0f};
                    PushFace(p, nrm, bt.back.x, bt.back.y, tw, th, is_water);
                }
                if (FaceVisible(block, BlockAt(c, x, y + 1, z))) {   // top (+Y)
                    float p[4][3] = {{cx-h,cy+h,cz+h},{cx+h,cy+h,cz+h},{cx+h,cy+h,cz-h},{cx-h,cy+h,cz-h}};
                    float nrm[3] = {0.0f, 1.0f, 0.0f};
                    PushFace(p, nrm, bt.top.x, bt.top.y, tw, th, is_water);
                }
                if (FaceVisible(block, BlockAt(c, x, y - 1, z))) {   // bottom (-Y)
                    float p[4][3] = {{cx-h,cy-h,cz-h},{cx+h,cy-h,cz-h},{cx+h,cy-h,cz+h},{cx-h,cy-h,cz+h}};
                    float nrm[3] = {0.0f, -1.0f, 0.0f};
                    PushFace(p, nrm, bt.bottom.x, bt.bottom.y, tw, th, is_water);
                }
                if (FaceVisible(block, BlockAt(c, x + 1, y, z))) {   // right (+X)
                    float p[4][3] = {{cx+h,cy-h,cz+h},{cx+h,cy-h,cz-h},{cx+h,cy+h,cz-h},{cx+h,cy+h,cz+h}};
                    float nrm[3] = {1.0f, 0.0f, 0.0f};
                    PushFace(p, nrm, bt.right.x, bt.right.y, tw, th, is_water);
                }
                if (FaceVisible(block, BlockAt(c, x - 1, y, z))) {   // left (-X)
                    float p[4][3] = {{cx-h,cy-h,cz-h},{cx-h,cy-h,cz+h},{cx-h,cy+h,cz+h},{cx-h,cy+h,cz-h}};
                    float nrm[3] = {-1.0f, 0.0f, 0.0f};
                    PushFace(p, nrm, bt.left.x, bt.left.y, tw, th, is_water);
                }
            }
        }
    }
}
