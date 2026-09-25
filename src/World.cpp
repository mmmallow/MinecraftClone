#include "World.h"
#include <rlgl.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cmath>

// Default raylib fragment shader, plus discarding fully transparent texels so
// they don't write depth and hide blocks behind them (e.g. through leaves).
static const char* CHUNK_FS = R"(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
out vec4 finalColor;
void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.5) discard;
    finalColor = texelColor*colDiffuse*fragColor;
}
)";

World::World (Player& player, int seed)
: player (player),
  seed (seed)
{
    texture_atlas = LoadTexture("assets/tex_atlas.png");
    MapBlockTextures();

    chunk_material = LoadMaterialDefault();
    chunk_material.shader = LoadShaderFromMemory(nullptr, CHUNK_FS);
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

    UnloadShader(chunk_material.shader);
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
        if (pos.x == chunk->position.x || pos.x == chunk->position.x + CHUNK_EDGE_LEN-1) {
            int sign = pos.x == chunk->position.x ? -1 : 1;
            auto neighbor_x = chunks.find(ChunkKey(chunk->grid_x+sign, chunk->grid_z));
            if (neighbor_x != chunks.end())
                neighbor_x->second->dirty = true;
        }
        if (pos.z == chunk->position.z || pos.z == chunk->position.z + CHUNK_EDGE_LEN-1) {
            int sign = pos.z == chunk->position.z ? -1 : 1;
            auto neighbor_z = chunks.find(ChunkKey(chunk->grid_x, chunk->grid_z+sign));
            if (neighbor_z != chunks.end())
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

        for (auto& pair : chunks) {
            Chunk* c = pair.second;
            if (c->dirty) RebuildMesh(c);
            if (!c->has_mesh) continue;
            DrawMesh(c->mesh, chunk_material, MatrixTranslate(c->position.x, c->position.y, c->position.z));
        }
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
    //seed = 777;
    int octaves = 5;
    PerlinNoise perlin { seed };
    srand(seed);

    // Generate height map
    int width = 16;
    int height = 16;
    float persistence = 0.5f;
    float lacunarity = 0.5f;
    float y_offset = 0;

    const float FREQUENCY = 0.03f; // noise units per block
    const int   BASE_Y    = 64;    // height a noise value of 0.5 maps to
    const int   AMPLITUDE = 32;    // how far terrain swings above/below BASE_Y

    int chunk_x = 0;
    int chunk_z = 0;


    std::vector<Vector3> leaves;
    std::vector<Vector3> logs;

    for (chunk_z = 0; chunk_z < height; chunk_z++) {
        for (chunk_x = 0; chunk_x < width; chunk_x++) {
            Chunk* c = new Chunk(chunk_x, chunk_z);
            chunks.insert({ChunkKey(c->grid_x, c->grid_z), c});

            std::vector<std::vector<float>> map(height, std::vector<float>(width));

            for (int z = 0; z < height; z++) {
                for (int x = 0; x < width; x++) {
                    float nx = (c->position.x + static_cast<float>(x)) * FREQUENCY;
                    float nz = (c->position.z + static_cast<float>(z)) * FREQUENCY;

                    map[z][x] = perlin.FractalNoise(nx, nz, y_offset,
                                                    octaves, persistence, lacunarity);
                }
            }

            int middle_y = BASE_Y;
            for (int z = 0; z < CHUNK_EDGE_LEN; z++) {
                for (int x = 0; x < CHUNK_EDGE_LEN; x++) {
                    // Generate surface blocks
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

                    // Generate trees
                    if (type == GRASS) {
                        int random_num = rand() % (900);
                        if (random_num == 899) {
                            c->SetFromWorld({ c->position.x + static_cast<float>(x),
                                            static_cast<float>(surface_y+1),
                                            c->position.z + static_cast<float>(z) }, LOG);
                            c->SetFromWorld({ c->position.x + static_cast<float>(x),
                                            static_cast<float>(surface_y+2),
                                            c->position.z + static_cast<float>(z) }, LOG);
                            c->SetFromWorld({ c->position.x + static_cast<float>(x),
                                            static_cast<float>(surface_y+3),
                                            c->position.z + static_cast<float>(z) }, LOG);

                            for (int i = 0; i < 3; i++) {
                                Vector3 direction = {uniform_random(-1.0f, 1.0f), 1.0f, uniform_random(-1.0f, 1.0f)};
                                Ray ray = { {c->position.x + static_cast<float>(x), static_cast<float>(surface_y+3), c->position.z + static_cast<float>(z)},
                                            Vector3Normalize(direction)};
                                auto vec = TreeGen(ray);
                                logs.insert(logs.end(), vec.begin(), vec.end());

                                auto leaf = LeafGen(logs.back());
                                leaves.insert(leaves.end(), leaf.begin(), leaf.end());
                            }


                        }
                    }

                    // Fill chunk with blocks
                    for (int y = surface_y-1; y >= 0; y--) {
                        type = DIRT;
                        if (y < 22) {
                            type = STONE;
                        }
                        c->SetFromWorld({ c->position.x + static_cast<float>(x),
                                        static_cast<float>(y),
                                        c->position.z + static_cast<float>(z) }, type);
                    }

                    if (z == CHUNK_EDGE_LEN / 2 && x == CHUNK_EDGE_LEN / 2)
                        middle_y = surface_y;
                }
            }

            if (chunk_z == height / 2 && chunk_x == width / 2) {
                player.position.x = chunk_x * width + CHUNK_EDGE_LEN / 2;
                player.position.y = middle_y + 10.0f;
                player.position.z = chunk_z * height + CHUNK_EDGE_LEN / 2;
            }
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
    int num_leaves = 110 + rand() % (140 - 110);
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

    auto it = chunks.find(ChunkKey(FloorDiv(wx, CHUNK_EDGE_LEN),
                                   FloorDiv(wz, CHUNK_EDGE_LEN)));
    if (it == chunks.end()) return true;   // outside the generated world

    Chunk* c = it->second;
    int lx = PosMod(wx, CHUNK_EDGE_LEN);
    int lz = PosMod(wz, CHUNK_EDGE_LEN);
    return BlockIsSeeThrough(c->data[lx + lz*CHUNK_EDGE_LEN + wy*CHUNK_AREA]);
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

                if (SeeThrough(c, x, y, z + 1)) {   // front (+Z)
                    float p[4][3] = {{cx-h,cy-h,cz+h},{cx+h,cy-h,cz+h},{cx+h,cy+h,cz+h},{cx-h,cy+h,cz+h}};
                    float nrm[3] = {0.0f, 0.0f, 1.0f};
                    PushFace(p, nrm, bt.front.x, bt.front.y, tw, th);
                }
                if (SeeThrough(c, x, y, z - 1)) {   // back (-Z)
                    float p[4][3] = {{cx+h,cy-h,cz-h},{cx-h,cy-h,cz-h},{cx-h,cy+h,cz-h},{cx+h,cy+h,cz-h}};
                    float nrm[3] = {0.0f, 0.0f, -1.0f};
                    PushFace(p, nrm, bt.back.x, bt.back.y, tw, th);
                }
                if (SeeThrough(c, x, y + 1, z)) {   // top (+Y)
                    float p[4][3] = {{cx-h,cy+h,cz+h},{cx+h,cy+h,cz+h},{cx+h,cy+h,cz-h},{cx-h,cy+h,cz-h}};
                    float nrm[3] = {0.0f, 1.0f, 0.0f};
                    PushFace(p, nrm, bt.top.x, bt.top.y, tw, th);
                }
                if (SeeThrough(c, x, y - 1, z)) {   // bottom (-Y)
                    float p[4][3] = {{cx-h,cy-h,cz-h},{cx+h,cy-h,cz-h},{cx+h,cy-h,cz+h},{cx-h,cy-h,cz+h}};
                    float nrm[3] = {0.0f, -1.0f, 0.0f};
                    PushFace(p, nrm, bt.bottom.x, bt.bottom.y, tw, th);
                }
                if (SeeThrough(c, x + 1, y, z)) {   // right (+X)
                    float p[4][3] = {{cx+h,cy-h,cz+h},{cx+h,cy-h,cz-h},{cx+h,cy+h,cz-h},{cx+h,cy+h,cz+h}};
                    float nrm[3] = {1.0f, 0.0f, 0.0f};
                    PushFace(p, nrm, bt.right.x, bt.right.y, tw, th);
                }
                if (SeeThrough(c, x - 1, y, z)) {   // left (-X)
                    float p[4][3] = {{cx-h,cy-h,cz-h},{cx-h,cy-h,cz+h},{cx-h,cy+h,cz+h},{cx-h,cy+h,cz-h}};
                    float nrm[3] = {-1.0f, 0.0f, 0.0f};
                    PushFace(p, nrm, bt.left.x, bt.left.y, tw, th);
                }
            }
        }
    }
}
