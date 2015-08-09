#version 140
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require
//#include <phong.glsl>
//**************************************************
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
  return light_attenuation_factor /
           (light_constant_attenuation +
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
    light_direction = normalize(light_direction);
    float NdotL = max(dot(light_direction, v_normal), 0.0);
    //only shade front face
    if (NdotL > 0.0) {
      float att = attenuation(1.0, light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_distance);

      vec3 half_direction = normalize(light_direction + view_direction);
      float specular_angle = max(dot(half_direction, v_normal), 0.0);
      float specular_intensity = pow(specular_angle, material_shininess_factor);

      result.diffuse = att * light_color * NdotL;
      result.specular = att * specular_intensity * light_color;
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
  light_direction = normalize(light_direction);
  float NdotL = max(dot(light_direction, v_normal), 0.0);
  //only shade front face
  if (NdotL > 0.0) {
    float light_attenuation_factor = dot(normalize(spot_direction), -light_direction);
    //if fragment is outside spot cone
    if (light_attenuation_factor > 0 || abs(light_attenuation_factor) < spot_cos_cutoff) { return result; }

    light_attenuation_factor = pow(light_attenuation_factor, spot_exponent);
    float att = attenuation(light_attenuation_factor, light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_distance);

    vec3 half_direction = normalize(light_direction + view_direction);
    float specular_angle = max(dot(half_direction, v_normal), 0.0);
    float specular_intensity = pow(specular_angle, material_shininess_factor);
    result.diffuse = att * NdotL * light_color;
    result.specular = att * specular_intensity * light_color;
  }
  else {
    result.diffuse = vec3(0);
    result.specular = vec3(0);
  }
  return result;
}

LightingInfo computeLighting(vec3 eye, vec3 coordinates, vec3 normal, Light light) {
  normal = normalize(normal);
  LightingInfo lighting;
  //spot light
  if (light.position.w > 2.5) {
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
//*****************************************************************
//END phong.glsl

#define MAX_LIGHTS 256

const float material_ambient_factor = 0.2;
const float material_diffuse_factor = 0.5;
const float material_specular_factor = 0.6;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform usampler3D clusterTexture;
uniform usampler2D lightIndicesTexture;

uniform mat4 transformInverted;
uniform vec3 eye;

uniform int enableDebugOutput;

in vec2 v_uv;
in vec2 v_screenCoordinates;

layout (location = 0) out vec4 fragColor;

layout (std140) uniform Lights
{
    vec4 ambient_color;
    uint number_of_lights;
    uint padding1;
    uint padding2;
    uint padding3;
    Light lights[MAX_LIGHTS];
		
};

vec3 base_color()
{
	return texture(colorTexture, v_uv).rgb;
}

void main()
{
    vec3 normal = texture(normalTexture, v_uv).xyz;
    float depth = texture(depthTexture, v_uv).r;
    // depth is in [0;1] whereas NDCs are in [-1;1]^3
    vec4 worldCoordinates = transformInverted * vec4(v_screenCoordinates, depth * 2.0 - 1.0, 1);
		worldCoordinates /= worldCoordinates.w;

  	vec3 ambient = ambient_color.rgb;
  	vec3 diffuse = vec3(0);
  	vec3 specular = vec3(0);

		uvec2 lookup = texture(clusterTexture, vec3(v_uv, depth)).xy;
  	for (uint li = 0u; li < lookup.y; ++li)
    {
			int ls = int(lookup.x + li);
			ivec2 lc = ivec2(ls % 4096, ls / 4096);
			uint i = texelFetch(lightIndicesTexture, lc, 0).x;

			LightingInfo lighting = computeLighting(eye, worldCoordinates.xyz, normal, lights[i]);

      diffuse += lighting.diffuse;
      specular += lighting.specular;
    }

  	vec3 combined_lighting = material_ambient_factor * ambient
                             + material_diffuse_factor * diffuse
                             + material_specular_factor * specular;

  	vec3 lighted_color = base_color() * min(combined_lighting, 1);

		if (v_screenCoordinates.x < 0 || enableDebugOutput == 0)
    	fragColor = vec4(lighted_color, 1);
		else
			fragColor = vec4(vec3(lookup.y) / 32.f, 1);

    gl_FragDepth = depth;
}
