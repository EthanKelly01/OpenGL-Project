#version 460 core

struct BufferData {
	mat4 transform;
	vec4 vColour;
};

uniform mat4 view;
uniform mat4 projection;

layout(std430, binding = 0) readonly buffer ssbo1 {
	BufferData bufferData[];
};

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexture;

out vec3 normal;
out vec4 colour;

void main() {
	gl_Position = projection * view * bufferData[gl_InstanceID].transform * vec4(vPosition, 1.0);
	normal = (view * vec4(vNormal, 0.0)).xyz;
	colour = bufferData[gl_InstanceID].vColour;
}