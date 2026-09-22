#ifndef WORLD_H
#define WORLD_H

#include <raylib.h>
#include <raymath.h>
#include <vector>
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
    void DrawCubeTextureRec(Rectangle source, Vector3 position, float width, float height, float length, Color color);

    Player& player;
    std::vector<Block> blocks;

    Texture2D texture_atlas;
};

#endif // !WORLD_H
