#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 projectionMat;
uniform mat4 modelViewMat;
uniform mat3 normalMat;

void main()
{
    vec4 viewPos = modelViewMat * vec4(position, 1.0f);
    FragPos = viewPos.xyz;
    gl_Position = projectionMat * viewPos;
    TexCoords = texCoords;
    Normal = normalMat * normal;
}
