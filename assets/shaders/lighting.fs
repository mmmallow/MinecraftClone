#version 330
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.5) discard;

    vec3 n = abs(fragNormal);
    float light;
    if (n.y > 0.5) light = fragNormal.y > 0.0 ? 1.0f : 0.5;
    else if (n.z > 0.5) light = 0.8;
    else light = 0.6;

    finalColor = vec4(texelColor.rgb * light, texelColor.a) * colDiffuse;
}
