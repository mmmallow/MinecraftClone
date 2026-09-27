#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;
uniform float time;

out vec2 fragTexCoord;
out vec3 fragNormal;

void main() {
    vec3 pos = vertexPosition;

    if (vertexNormal.y > 0.5) {
        vec3 world = (matModel * vec4(pos, 1.0f)).xyz;
        float wave = sin(world.x * 0.8 + time * 1.5) + sin(world.z * 0.6 + time * 1.1);
        pos.y += (wave * 0.5 - 1.0) * 0.06;
    }

    fragTexCoord = vertexTexCoord;
    fragNormal = vertexNormal;
    gl_Position = mvp * vec4(pos, 1.0);
}
