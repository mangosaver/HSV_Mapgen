#version 330 core
out vec4 color;

in vec2 TexCoord;
in vec3 outColor;

uniform sampler2D texImage;

void main() {
    vec4 texColor = texture(texImage, TexCoord);
    color = texColor;
}