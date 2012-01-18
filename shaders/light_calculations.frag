float calcAttenuation(in vec3 world_pos,in vec3 light_pos, inout vec3 light_dir) {
	float lightDistanceSqr = dot(light_dir, light_dir);
	light_dir *= inversesqrt(lightDistanceSqr);
	
	return (1 / ( 1.0 + Lgt.light_attenuation * sqrt(lightDistanceSqr)));
}


vec4 computeLighting(in light_data light, in vec4 originalColor, in vec3 surfaceNormal) {
	vec3  light_dir = vec3(light.position) - world_pos;
	vec4 lightIntensity;
	//Turn off light attenuatin if w == 0.0
	if(light.position.w == 0.0) {
		lightIntensity = light.intensity;	
	} else { 
		float atten = calcAttenuation(world_pos, vec3(light.position), light_dir);
		lightIntensity =  atten * light.intensity;
	}

	float cosAngIncidence = dot(surfaceNormal, light_dir);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);

	vec3 viewDirection = normalize(camera_pos-world_pos);
	vec3 halfAngle = normalize(light_dir + viewDirection);
	float angleNormalHalf = acos(dot(halfAngle, surfaceNormal));
	float exponent = angleNormalHalf / Mtl.shininess;
	exponent = -(exponent * exponent);
	float gaussianTerm = exp(exponent);

	gaussianTerm = cosAngIncidence != 0.0 ? gaussianTerm : 0.0;
	
	return (originalColor*lightIntensity*cosAngIncidence) + (Mtl.specular * lightIntensity * gaussianTerm);
}
