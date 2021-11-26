#version 450 core
layout (location = 0) out vec4 gPositionDepth;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform float NEAR;
uniform float FAR;
float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
} // https://learnopengl.com/Advanced-OpenGL/Depth-testing

void main() {
    gPositionDepth.xyz = FragPos;
    gPositionDepth.w = LinearizeDepth(gl_FragCoord.z);
    gNormal = normalize(Normal);
    gAlbedo = vec3(0.95);
}
