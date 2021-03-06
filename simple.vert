#version 330

uniform mat4 mvp;

layout (location = 0) in vec4 position;

out vec4 vert_color;

void main() {
	gl_Position = mvp * position;
	vert_color = vec4(normalize(vec3(position)), 1.0)+vec4(0.2, 0.2,0.2,0.0);
}

