#version 330 core
uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexture;
in vec4 vColour;

out vec4 colour;
out vec3 normal;

void main() {
	gl_Position = projection * view * transform * vec4(vPosition, 1.0);
	//gl_Position = gl_Position + 0.2 * gl_InstanceID;
	normal = (view * vec4(vNormal, 0.0)).xyz;
	colour = vColour;
}