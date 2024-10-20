#version 330 core
in vec3 normal;
in vec4 colour;

void main() {
	vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
	vec3 L = vec3(1.0, 0.0, 0.0);
	vec3 H = normalize(L + vec3(0.0, 0.0, 1.0));
	vec3 N = normalize(normal);
	float diffuse = dot(N, L);

	float specular;
 	if (diffuse < 0.0) {
		diffuse = 0.0;
		specular = 0.0;
	} else specular = pow(max(0.0, dot(N, H)), 100.0);

	gl_FragColor = min(0.3*colour + 0.7*diffuse*colour + 0.7*white*specular, vec4(1.0));
	gl_FragColor.a = colour.a;
}