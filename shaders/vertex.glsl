#version 330 core

out vec2 TexCoord;

//in vec2 position;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec2 vertexPosition_modelspace;
layout (location = 1) in vec2 aTexCoord;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main() {
    // Output position of the vertex, in clip space : MVP * position
    TexCoord = aTexCoord;
    gl_Position = MVP * vec4(vertexPosition_modelspace, 0, 1);
}