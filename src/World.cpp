#include "World.h"
#include <rlgl.h>
#include <ctime>
#include <cstdlib>
#include <iostream>

World::World (Player& player, int seed)
: player (player),
  seed (seed)
{
    texture_atlas = LoadTexture("assets/tex_atlas.png");
    MapBlockTextures();

    Generate();
}

World::~World() {

}

void World::Destroy() {
    UnloadTexture(texture_atlas);
    for (auto& pair : chunks) {
        pair.second->Destroy();
        delete pair.second;
    }
}


void World::Update() {
    player.Update(GetFrameTime());

    /*if (!player.is_freecam) {
        for (auto it = blocks.begin(); it != blocks.end(); it++) {
            if (it->is_solid)
                CheckCollisions(*it);
        }
    }*/
}

void World::Draw() {
    bool draw_wires = IsKeyDown(KEY_P) ? true : false;
    ClearBackground(SKYBLUE);
    BeginMode3D(player.camera);
        if (draw_wires)
            rlEnableWireMode();
        else
            rlDisableWireMode();

        for (int i = 0; i < next_chunk_id; i++) {
            Chunk* cur = chunks[i];
            for (int x = 0; x < cur->EDGE_LEN; x++) {
                for (int y = 0; y < cur->Y_LEN; y++) {
                    for (int z = 0; z < cur->EDGE_LEN; z++) {
                        Block block = cur->Get({x, y, z});
                        Vector2 world_xz = Vector2Add({cur->position.x, cur->position.z}, {x, z});
                        if (BlockIsSolid(block))
                            DrawCubeTextureRec(block_textures[block], {world_xz.x, y, world_xz.y}, BLOCK_SIZE.x, BLOCK_SIZE.y, BLOCK_SIZE.z, WHITE);
                    }
                }
            }
        }
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
}

void World::DrawCubeTextureRec(BlockTexture block_tex, Vector3 position, float width, float height, float length, Color color) {
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float texWidth = (float)texture_atlas.width;
    float texHeight = (float)texture_atlas.height;

    Rectangle source = {block_tex.front.x, block_tex.front.y, block_tex.size.x, block_tex.size.y};

    // Set desired texture to be enabled while drawing following vertex data
    rlSetTexture(texture_atlas.id);

    // We calculate the normalized texture coordinates for the desired texture-source-rectangle
    // It means converting from (tex.width, tex.height) coordinates to [0.0f, 1.0f] equivalent
    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Front face
        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z + length/2);

        source.x = block_tex.back.x;
        source.y = block_tex.back.y;
        // Back face
        rlNormal3f(0.0f, 0.0f, - 1.0f);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z - length/2);

        source.x = block_tex.top.x;
        source.y = block_tex.top.y;
        // Top face
        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y + height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z - length/2);

        source.x = block_tex.bottom.x;
        source.y = block_tex.bottom.y;
        // Bottom face
        rlNormal3f(0.0f, - 1.0f, 0.0f);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y - height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z + length/2);

        source.x = block_tex.right.x;
        source.y = block_tex.right.y;
        // Right face
        rlNormal3f(1.0f, 0.0f, 0.0f);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z - length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z - length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x + width/2, y + height/2, z + length/2);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x + width/2, y - height/2, z + length/2);

        source.x = block_tex.left.x;
        source.y = block_tex.left.y;
        // Left face
        rlNormal3f( - 1.0f, 0.0f, 0.0f);
        rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z - length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
        rlVertex3f(x - width/2, y - height/2, z + length/2);
        rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z + length/2);
        rlTexCoord2f(source.x/texWidth, source.y/texHeight);
        rlVertex3f(x - width/2, y + height/2, z - length/2);

    rlEnd();

    rlSetTexture(0);
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


void World::Generate() {
    if (seed == 0) {
        std::srand(std::time(0));
        seed = std::rand() * 2;
    }
    int octaves = 5;
    PerlinNoise perlin { 777 };

    // Generate height map
    int width = 16;
    int height = 16;
    float persistence = 0.01f;
    float lacunarity = 2.0f;
    float y_offset = 0;

    const float FREQUENCY = 0.02f; // noise units per block
    const int   BASE_Y    = 64;    // height a noise value of 0.5 maps to
    const int   AMPLITUDE = 32;    // how far terrain swings above/below BASE_Y

    int chunk_x = 0;
    int chunk_z = 0;

    for (chunk_z = 0; chunk_z < height; chunk_z++) {
        for (chunk_x = 0; chunk_x < width; chunk_x++) {
            Chunk* c = new Chunk(next_chunk_id, { static_cast<float>(chunk_x * width),
                                                0.0f,
                                                static_cast<float>(chunk_z * height) });
            chunks.insert({c->id, c});
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

            for (int z = 0; z < height; z++) {
                for (int x = 0; x < width; x++) {
                    int surface_y = BASE_Y + static_cast<int>((map[z][x] - 0.5f) * 2.0f * AMPLITUDE);
                    c->SetFromWorld({ c->position.x + static_cast<float>(x),
                                    static_cast<float>(surface_y),
                                    c->position.z + static_cast<float>(z) }, GRASS);
                }
            }
        }
    }
}
