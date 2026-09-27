#version 330
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D texture0;

out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    finalColor = vec4(texelColor.rgb, texelColor.a*0.8);
}
