#version 450 core
layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 projectionMat;
uniform mat4 viewMat; // no translation


void main() {
    vec4 pos = projectionMat * viewMat * vec4(position, 1.0);
    gl_Position = pos.xyww;
    TexCoords = position;
}
