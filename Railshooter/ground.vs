#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float zCoord; // Output to fragment shader

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    zCoord = aPos.z; // Pass the z-coordinate
}