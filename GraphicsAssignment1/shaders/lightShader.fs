#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec4 position;
//in vec2 uv;

uniform vec3 eye;
uniform vec4 colour;

//uniform sampler2D material0;
//uniform sampler2D material1;

void main() {
	vec3 N = normalize(normal); //light source
	vec3 Lposition = vec3(500.0, 500.0, 800.0); //light position
	vec4 Lcolour = vec4(1.0, 1.0, 1.0, 1.0); //light colour
	float n = 100.0; //light level
	vec3 L = normalize(Lposition - position.xyz);
	vec3 e = normalize(eye - position.xyz);
	vec3 H = normalize(L + e); //light + vec to eye + length
	float diffuse = dot(N, L);
	float specular;
	
	if (diffuse < 0.0) {
		diffuse = 0.0;
		specular = 0.0;
	} else {
		specular = pow(max(0.0, dot(N, H)), n);
	}

	FragColor = min(0.3*colour + diffuse*colour*Lcolour + Lcolour*specular, vec4(1.0));
	FragColor.a = colour.a;

	//FragColor = mix(texture(material0, texCoord), texture(material1, texCoord), 0.5);
}