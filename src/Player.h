#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>

class Player {
public:
    Camera3D camera;

    Player(Vector3 pos);
    ~Player();
    void HandleInput(KeyboardKey input);
    void Update(float dt);
    void ApplyPosition(float dt);
    void Draw();

    bool is_grounded = false;
    bool is_freecam = true;
    Vector3 size = {1.0f, 2.0f, 1.0f};
    Vector3 position;
    Vector3 velocity = { 0 };
    Vector3 dir = { 0 };

    const float MAX_SPEED = 10.0f;
    const float JUMP_FORCE = 10.0f;
    const float MAX_ACCEL = 100.0f;
    const float FRICTION = 0.76f;
    const float AIR_DRAG = 0.78f;
    const float CONTROL = 15.0f;

    const float STAND_HEIGHT = 0.5f;
    const float BOTTOM_HEIGHT = 0.0f;

    const float gravity = 32.0f;
    float head_timer = 0.0f;

    float walk_lerp = 0.0f;
    float head_lerp = STAND_HEIGHT;
    Vector2 look_rotation = { 0 };
    Vector2 lean = { 0 };
    Vector2 sensitivity = { 0.001f, 0.001f };

private:
    void UpdateCameraFPS();
    void UpdateBody(float dt, float rot, char side, char forward, bool jump_pressed);
};

#endif // !PLAYER_H
