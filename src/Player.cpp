#include "Player.h"
#include <raymath.h>
#include <algorithm>
#include <cmath>
#include <iostream>

Player::Player(Vector3 pos) {
    position = pos;
    camera.position = { pos.x, pos.y + BOTTOM_HEIGHT + head_lerp, pos.z };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = (Vector3){
        position.x,
        position.y + (BOTTOM_HEIGHT + head_lerp),
        position.z,
    };
    now = std::time(NULL);

    UpdateCameraFPS();
}

Player::~Player() {

}

void Player::HandleInput() {
    if (IsKeyPressed(KEY_C) && !is_freecam)
        is_freecam = true;
    else if (IsKeyPressed(KEY_C) && is_freecam)
        is_freecam = false;

    // very basic block breaking
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        world->SetBlock(current_block_looking.position, Block::AIR);

    float m = GetMouseWheelMove();
    if (m == 1)
        increase_block(current_block_held);
    else if (m == -1)
        decrease_block(current_block_held);

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && current_block_looking.position.x != 0.0f && current_block_looking.position.y != 256.0f && current_block_looking.position.z != 0.0f && current_block_held != NONE && current_block_held != AIR)
        world->SetBlock(Vector3Add(current_block_looking.position, current_block_looking.direction), current_block_held);
}

void Player::Update (float dt) {
    HandleInput();

    if (!is_freecam) {
        Vector2 mouse_delta = GetMouseDelta();
        look_rotation.x -= mouse_delta.x * sensitivity.x;
        look_rotation.y += mouse_delta.y * sensitivity.y;

        char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
        char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
        UpdateBody(dt, look_rotation.x, sideway, forward, IsKeyPressed(KEY_SPACE));

        head_lerp = Lerp(head_lerp, STAND_HEIGHT, 20.0f*dt);
        camera.position = (Vector3){
            position.x,
            position.y + (BOTTOM_HEIGHT + head_lerp),
            position.z,
        };

        if (is_grounded && ((forward != 0) || (sideway != 0))) {
            head_timer += dt * 3.0f;
            walk_lerp = Lerp(walk_lerp, 1.0f, 10.0f*dt);
            camera.fovy = Lerp(camera.fovy, 55.0f, 5.0f*dt);
        }
        else {
            walk_lerp = Lerp(walk_lerp, 0.0f, 10.0f*dt);
            camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f*dt);
        }

        lean.x = Lerp(lean.x, sideway*0.02f, 10.0f*dt);
        lean.y = Lerp(lean.y, forward*0.015f, 10.0f*dt);

        UpdateCameraFPS();
    }
    else {
        UpdateCamera(&camera, CAMERA_FREE);
        position = camera.position;
    }

    UpdateCurrentBlock();
    is_grounded = false;
}

void Player::UpdateCameraFPS() {
    const Vector3 up = (Vector3){ 0.0f, 1.0f, 0.0f };
    const Vector3 targetOffset = (Vector3){ 0.0f, 0.0f, -1.0f };

    Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, look_rotation.x);

    float max_angle_up = Vector3Angle(up, yaw);
    max_angle_up -= 0.001f;
    if ( -(look_rotation.y) > max_angle_up) { look_rotation.y = -max_angle_up; }

    float max_angle_down = Vector3Angle(Vector3Negate(up), yaw);
    max_angle_down *= -1.0f;
    max_angle_down += 0.001f;
    if ( -(look_rotation.y) < max_angle_down) { look_rotation.y = -max_angle_down; }

    Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

    float pitch_angle = -look_rotation.y - lean.y;
    pitch_angle = Clamp(pitch_angle, -PI/2 + 0.0001f, PI/2 - 0.0001f);
    Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitch_angle);

    float head_sin = sinf(head_timer*PI);
    float head_cos = cosf(head_timer*PI);
    const float step_rotation = 0.01f;
    camera.up = Vector3RotateByAxisAngle(up, pitch, head_sin*step_rotation + lean.x);

    const float bob_side = 0.1f;
    const float bob_up = 0.15f;
    Vector3 bobbing = Vector3Scale(right, head_sin*bob_side);
    bobbing.y = fabsf(head_cos*bob_up);

    camera.position = Vector3Add(camera.position, Vector3Scale(bobbing, walk_lerp));
    camera.target = Vector3Add(camera.position, pitch);
}

void Player::UpdateBody(float dt, float rot, char side, char forward, bool jump_pressed) {
    Vector2 input = (Vector2){ (float)side, (float)-forward };

    if ((side != 0) && (forward != 0)) input = Vector2Normalize(input);

    velocity.y -= gravity*dt;

    if (is_grounded && jump_pressed) {
        velocity.y = JUMP_FORCE;
        is_grounded = false;
    }

    Vector3 front = (Vector3){ sinf(rot), 0.0f, cosf(rot) };
    Vector3 right = (Vector3){ cosf(-rot), 0.0f, sinf(-rot) };

    Vector3 desired_dir = (Vector3){ input.x*right.x + input.y*front.x, 0.0f, input.x*right.z + input.y*front.z, };
    dir = Vector3Lerp(dir, desired_dir, CONTROL*dt);

    float decel = (is_grounded ? FRICTION : AIR_DRAG);
    Vector3 hvel = (Vector3){ velocity.x*decel, 0.0f, velocity.z*decel };

    float hvel_length = Vector3Length(hvel);
    if (hvel_length < (MAX_SPEED*0.01f)) hvel = { 0 };

    float speed = Vector3DotProduct(hvel, dir);
    float accel = Clamp(MAX_SPEED - speed, 0.0f, MAX_ACCEL*dt);
    hvel.x += dir.x*accel;
    hvel.z += dir.z*accel;

    velocity.x = hvel.x;
    velocity.z = hvel.z;

    position.x += velocity.x*dt;
    position.y += velocity.y*dt;
    position.z += velocity.z*dt;
}


/*
*
* DDA algo implementation comes from https://lodev.org/cgtutor/raycasting.html
*
*/
void Player::UpdateCurrentBlock() {
    if (std::time(NULL) >= now+5) {
        now = std::time(NULL);
    }

    Vector2 screen_center =  { GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    Ray ray = GetScreenToWorldRay(screen_center, camera);

    // (block n spans [n-0.5, n+0.5]), so shift by 0.5 to line up with the DDA grid
    Vector3 origin = Vector3AddValue(ray.position, 0.5f);

    int map_x = (int)std::floor(origin.x);
    int map_y = (int)std::floor(origin.y);
    int map_z = (int)std::floor(origin.z);
    int num_blocks = 0; // track the number of blocks checked to cap player reach

    // length of ray from current pos to next x or y side
    double side_dist_x;
    double side_dist_y;
    double side_dist_z;

    // length of ray from one x or y side to the next
    double delta_dist_x = (ray.direction.x == 0) ? 1e30 : std::abs(1 / ray.direction.x);
    double delta_dist_y = (ray.direction.y == 0) ? 1e30 : std::abs(1 / ray.direction.y);
    double delta_dist_z = (ray.direction.z == 0) ? 1e30 : std::abs(1 / ray.direction.z);
    double perp_block_dist;

    int step_x;
    int step_y;
    int step_z;

    int side;

    if (ray.direction.x < 0) {
        step_x = -1;
        side_dist_x = (origin.x - map_x) * delta_dist_x;
    }
    else {
        step_x = 1;
        side_dist_x = (map_x + 1.0f - origin.x) * delta_dist_x;
    }
    if (ray.direction.y < 0) {
        step_y = -1;
        side_dist_y = (origin.y - map_y) * delta_dist_y;
    }
    else {
        step_y = 1;
        side_dist_y = (map_y + 1.0f - origin.y) * delta_dist_y;
    }
    if (ray.direction.z < 0) {
        step_z = -1;
        side_dist_z = (origin.z - map_z) * delta_dist_z;
    }
    else {
        step_z = 1;
        side_dist_z = (map_z + 1.0f - origin.z) * delta_dist_z;
    }


    while (num_blocks <= 10) {
        if (side_dist_x < side_dist_y && side_dist_x < side_dist_z) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
            side = 0;
        }
        else if (side_dist_z < side_dist_x && side_dist_z < side_dist_y) {
            side_dist_z += delta_dist_z;
            map_z += step_z;
            side = 1;
        }
        else {
            side_dist_y += delta_dist_y;
            map_y += step_y;
            side = 2;
        }

        if (world->SolidAtWorld(map_x, map_y, map_z)) {
            if (side == 0) {
                perp_block_dist = side_dist_x - delta_dist_x;
                current_block_looking.direction = { static_cast<float>(-step_x), 0.0f, 0.0f };
            }
            else if (side == 1) {
                perp_block_dist = side_dist_z - delta_dist_z;
                current_block_looking.direction = { 0.0f, 0.0f, static_cast<float>(-step_z) };
            }
            else {
                perp_block_dist = side_dist_y - delta_dist_y;
                current_block_looking.direction = { 0.0f, static_cast<float>(-step_y), 0.0f };
            }

            if (perp_block_dist <= reach) {
                current_block_looking.position = { static_cast<float>(map_x), static_cast<float>(map_y), static_cast<float>(map_z) };
                return;
            }
            else num_blocks = 10;
        }

        num_blocks++;
    }

    // No block found
    current_block_looking.position = { 0.0f, 256.0f, 0.0f }; // arbitrary value player should never be able to reach
    current_block_looking.direction = { 0.0f, 0.0f, 0.0f };
}
