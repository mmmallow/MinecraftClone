#include "World.h"
#include <rlgl.h>

World::World (Player& player)
: player (player)
{
    texture_atlas = LoadTexture("assets/tex_atlas.png");

    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
            blocks.push_back({{i,-1.0f,j}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});
        }
    }
    blocks.push_back({{3.0f, 0.0f, 3.0f}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});
    blocks.push_back({{4.0f, 1.0f, 3.0f}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});

    blocks.push_back({{-2.0f, 0.0f, -2.0f}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});
    blocks.push_back({{-2.0f, 1.0f, -2.0f}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});
    blocks.push_back({{-4.0f, 0.0f, -2.0f}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});
    blocks.push_back({{-4.0f, 1.0f, -2.0f}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});
    blocks.push_back({{-3.0f, 2.0f, -2.0f}, { 16.0f, 0.0f, 16.0f, 16.0f }, DIRT, true});
}

World::~World() {

}

void World::Destroy() {
    UnloadTexture(texture_atlas);
}

void World::Update() {
    player.Update(GetFrameTime());

    for (auto it = blocks.begin(); it != blocks.end(); it++) {
        CheckCollisions(*it);
    }
}

void World::Draw() {
    ClearBackground(SKYBLUE);
    BeginMode3D(player.camera);
        for (auto block : blocks) {
            DrawCubeTextureRec(block.tex_pos, block.position, BLOCK_SIZE.x, BLOCK_SIZE.y, BLOCK_SIZE.z, WHITE);
            //DrawCubeWires(block.position, 1.0f, 1.0f, 1.0f, BLACK);
        }
    EndMode3D();
}


void World::DrawCubeTextureRec(Rectangle source, Vector3 position, float width, float height, float length, Color color) {
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float texWidth = (float)texture_atlas.width;
    float texHeight = (float)texture_atlas.height;

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

void World::CheckCollisions (Block block) {
    BoundingBox player_box = GetBoundingBox(player.position, player.size);
    BoundingBox cube_box = GetBoundingBox(block.position, BLOCK_SIZE);

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
