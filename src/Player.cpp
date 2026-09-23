#include "Player.h"
#include <raymath.h>

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

    UpdateCameraFPS();
}

Player::~Player() {

}

void Player::Update (float dt) {
    if (IsKeyPressed(KEY_C) && !is_freecam)
        is_freecam = true;
    else if (IsKeyPressed(KEY_C) && is_freecam)
        is_freecam = false;

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
