const float material_shininess_factor = 16;

struct Light {
	vec4 position;
	vec4 color;
  vec4 attenuation;
  vec4 multiuse;
};

struct LightingInfo {
  vec3 diffuse;
  vec3 specular;
};

float attenuation(
float light_attenuation_factor,
float light_constant_attenuation,
float light_linear_attenuation,
float light_quadratic_attenuation,
float light_distance) {
  return light_attenuation_factor / (light_constant_attenuation +
                  light_linear_attenuation * light_distance +
                  light_quadratic_attenuation * light_distance * light_distance);
}

LightingInfo unidirectionalLighting(
vec3 v_eye,
vec3 worldCoordinates,
vec3 v_normal,
vec3 light_position,
vec3 light_color,
float light_constant_attenuation,
float light_linear_attenuation,
float light_quadratic_attenuation,
float material_shininess_factor
) {
    LightingInfo result;
    vec3 view_direction = normalize(v_eye - worldCoordinates);
    vec3 light_direction = light_position - worldCoordinates;
    float light_distance = length(light_direction);
		float NdotL = max(dot(normalize(light_direction), v_normal), 0.0);
    //only shade front face
    if (NdotL > 0.0) {
      float att = attenuation(1.0, light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_distance);

      vec3 half_direction = normalize(light_direction + view_direction);
      float specular_angle = max(dot(half_direction, v_normal), 0.0);
      float specular_intensity = pow(specular_angle, material_shininess_factor);

      result.diffuse = att * light_color * NdotL;
      result.specular = vec3(att * specular_intensity * light_color);
    }
    else {
      result.diffuse = vec3(0);
      result.specular = vec3(0);
    }
    return result;
}

LightingInfo spotLighting(
vec3 v_eye,
vec3 worldCoordinates,
vec3 v_normal,
vec3 light_position,
vec3 light_color,
float light_constant_attenuation,
float light_linear_attenuation,
float light_quadratic_attenuation,
float spot_cos_cutoff,
float spot_exponent,
vec3  spot_direction,
float material_shininess_factor
) {
  LightingInfo result;
  vec3 view_direction = normalize(v_eye - worldCoordinates);
  vec3 light_direction = light_position - worldCoordinates;
  float light_distance = length(light_direction);
  float NdotL = max(dot(normalize(light_direction), v_normal), 0.0);
  //only shade front face
  if (NdotL > 0.0) {
    float light_attenuation_factor = dot(normalize(spot_direction), normalize(-light_direction));
    //if fragment is outside spot cone
    if (light_attenuation_factor > 0 || abs(light_attenuation_factor) < spot_cos_cutoff) { return result; }

    light_attenuation_factor = pow(light_attenuation_factor, spot_exponent);
    float att = attenuation(light_attenuation_factor, light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_distance);

    vec3 half_direction = normalize(light_direction + view_direction);
    float specular_angle = max(dot(half_direction, v_normal), 0.0);
    float specular_intensity = pow(specular_angle, material_shininess_factor);
    result.diffuse = att * light_color * NdotL;
    result.specular = vec3(light_color);
  }
  else {
    result.diffuse = vec3(0);
    result.specular = vec3(0);
  }
  return result;
}

LightingInfo computeLighting(vec3 eye, vec3 coordinates, vec3 normal, Light light) {
  LightingInfo lighting;
  //spot light
  if (light.position.w > 2.0) {
    lighting = spotLighting(
      eye,
      coordinates,
      normal,
      light.position.xyz,
      light.color.rgb,
      light.attenuation.x,
      light.attenuation.y,
      light.attenuation.z,
      light.multiuse.w,
      light.attenuation.w,
      light.multiuse.xyz,
      material_shininess_factor
    );
  }
  //uni
  else {
    lighting = unidirectionalLighting(
      eye,
      coordinates,
      normal,
      light.position.xyz,
      light.color.rgb,
      light.attenuation.x,
      light.attenuation.y,
      light.attenuation.z,
      material_shininess_factor
    );
  }

  return lighting;
}
