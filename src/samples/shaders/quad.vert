#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec2 inUv;

layout (location = 0) out vec2 outUv;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	outUv = inUv;
	gl_Position = vec4(inPosition, 1.0);
}
