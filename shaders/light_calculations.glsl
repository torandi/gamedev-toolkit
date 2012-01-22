vec4 computeLighting(
	in light_data light, in vec4 originalColor, 
	in vec3 normal_map, in vec3 light_dir, 
	in vec3 camera_dir, in vec3 light_distance
	) {
	vec3 lightIntensity;

	float specular_intensity = 1.0;

	//Turn off light attenuation if w == 0.0
	if(light.position.w == 0.0) {
		lightIntensity = light.intensity.rgb;	
	} else { 
		float lightAttenuation = (1 / ( 1.0 + light.attenuation * length(light_distance)));
		lightIntensity =  lightAttenuation * light.intensity.rgb;
	}

	float LambertTerm = max( dot(light_dir, normal_map), 0.0);
	float specular_amount = 0.0;

	if( LambertTerm > 0.0) {
		//Apply specular
		specular_amount = pow(clamp(dot(reflect(light_dir, normal_map), camera_dir), 0.0, 1.0), Mtl.shininess);
	}

	vec3 diffuse = originalColor.rgb * LambertTerm * lightIntensity;	
	vec3 specular = Mtl.specular.rgb * specular_amount * specular_intensity * length(diffuse);

	vec4 color;
	color.rgb = diffuse + specular;
	color.a = 1.0;

	return color;
/*
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
*/	
}
