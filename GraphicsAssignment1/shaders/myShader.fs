#version 330 core
out vec4 FragColor;

in vec3 pos;
in vec3 normal;
in vec2 texCoord;

uniform vec3 lightPos;

uniform sampler2D material0;
uniform sampler2D material1;

void main() {
	float ambient = 0.3;
	vec4 lightColour = vec4(1.0, 1.0, 1.0, 1.0);

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - pos);
	float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = diff * lightColour;

	//FragColor = (ambient + diff) * lightColour * mix(texture(material0, texCoord), texture(material1, texCoord), 0.5);
	FragColor = (ambient + diff) * lightColour;
}