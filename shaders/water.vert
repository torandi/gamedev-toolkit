#version 330
#include "uniforms.glsl"

#define MAX_NUM_WAVES 8

const float pi = 3.14159;
uniform int num_waves;
uniform float time;
uniform float amplitude[MAX_NUM_WAVES];
uniform float wavelength[MAX_NUM_WAVES];
uniform float speed[MAX_NUM_WAVES];
uniform vec2 direction[MAX_NUM_WAVES];

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec4 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_bitangent;

out vec3 position;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;
out vec2 texcoord;

float wave(int i, vec2 pos) {
	float frequency = 2*pi/wavelength[i];
	float phase = speed[i] * frequency;
	float theta = dot(direction[i], pos);
	return amplitude[i] * sin(theta * frequency + time*phase);
}

float waveHeight(vec2 pos) {
	float height = 0.0;
	for(int i=0; i<num_waves; ++i) {
		height += wave(i, pos);
	}
	return height;
}

float dWavedx(int i, vec2 pos) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], pos);
    float A = amplitude[i] * direction[i].x * frequency;
    return A * cos(theta * frequency + time * phase);
}

float dWavedy(int i, vec2 pos) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], pos);
    float A = amplitude[i] * direction[i].y * frequency;
    return A * cos(theta * frequency + time * phase);
}

vec3 waveNormal(vec2 pos) {
    float dx = 0.0;
    float dy = 0.0;
    for (int i = 0; i < num_waves; ++i) {
        dx += dWavedx(i, pos);
        dy += dWavedy(i, pos);
    }
    vec3 n = vec3(-dx, 1.0, -dy);
    return normalize(n);
}

void main() {
	vec4 pos = in_position;
	pos.y+= waveHeight(pos.xz);
	vec4 w_pos = modelMatrix * pos;
	position = w_pos.xyz;
	gl_Position = projectionViewMatrix *  w_pos;
	texcoord = in_texcoord;
	normal = (normalMatrix * (vec4(waveNormal(pos.xz), 1.0))).xyz;
	tangent = (normalMatrix * in_tangent).xyz;
	bitangent = (normalMatrix * in_bitangent).xyz;
}

