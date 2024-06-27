#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseTexture;

void main()
{
	// outFragColor = vec4(inColor, 1.0);
	 outFragColor = texture(diffuseTexture, inUV);
}
