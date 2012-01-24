vec3 skybox_color(vec3 texcoord) {
	vec4 tex_color = textureCube(skybox, texcoord.xyz);
	return tex_color.rgb*Lgt.ambient_intensity.rgb*(1.0f+tex_color.a);
}
