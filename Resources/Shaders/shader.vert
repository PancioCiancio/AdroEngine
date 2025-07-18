#version 450

layout (set = 0, binding = 0) uniform transforms_ {
    mat4 view;
    mat4 projection;
} transforms;


layout (location = 0) in vec3 positions;
layout (location = 1) in vec3 colors;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = transforms.projection * transforms.view * vec4(positions, 1.0);
    fragColor = colors;
}