#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUv;
layout (location = 3) in vec3 inNormal;

layout (set = 1, binding = 0) uniform UBO
{
	mat4 projection;
	mat4 camera;
} cmaeraView;

layout (push_constant) uniform PC
{
	layout (offset=0) mat4  model;
} movelView;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUv;

out gl_PerVertex
{
    vec4 gl_Position;
};


void main()
{
	outColor = inColor;
	outUv = inUv;
	gl_Position = cmaeraView.projection * cmaeraView.camera * movelView.model * vec4(inPosition, 1.0);
}
