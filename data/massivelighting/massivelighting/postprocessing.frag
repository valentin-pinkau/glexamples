#version 140
#extension GL_ARB_explicit_attrib_location : require

const float material_ambient_factor = 0.2;
const float material_diffuse_factor = 0.5;
const float material_specular_factor = 0.6;
const float material_shininess_factor = 16;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform sampler3D clusterTexture;

uniform mat4 transformInverted;
uniform vec3 eye;
uniform ivec2 [] lightIndices;

in vec2 v_uv;
in vec2 v_screenCoordinates;

layout (location = 0) out vec4 fragColor;

#define MAX_LIGHTS 256

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

layout (std140) uniform Lights
{
		vec4 ambient_color;
		Light lights[MAX_LIGHTS];
		uint number_of_lights;
};

vec3 base_color()
{
	return texture(colorTexture, v_uv).rgb;
}

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

void main()
{
    vec3 normal = texture(normalTexture, v_uv).xyz;
    float depth = texture(depthTexture, v_uv).r;
    // depth is in [0;1] whereas NDCs are in [-1;1]^3
    vec4 almostWorldCoordinates = transformInverted * vec4(v_screenCoordinates, depth * 2.0 - 1.0, 1);
    vec4 worldCoordinates = vec4(almostWorldCoordinates.xyz / almostWorldCoordinates.w, 1);


  	vec3 ambient = ambient_color.rgb;
  	vec3 diffuse = vec3(0);
  	vec3 specular = vec3(0);

  	for (uint i = 0u; i < number_of_lights; ++i)
    {
      LightingInfo lighting;
      //spot light
      if (lights[i].position.w > 2.0) {
        lighting = spotLighting(
          eye,
          worldCoordinates.xyz,
          normal,
          lights[i].position.xyz,
          lights[i].color.rgb,
          lights[i].attenuation.x,
          lights[i].attenuation.y,
          lights[i].attenuation.z,
          lights[i].multiuse.w,
          lights[i].attenuation.w,
          lights[i].multiuse.xyz,
          material_shininess_factor
        );
      }
      //uni
      else {
        lighting = unidirectionalLighting(
          eye,
          worldCoordinates.xyz,
          normal,
          lights[i].position.xyz,
          lights[i].color.rgb,
          lights[i].attenuation.x,
          lights[i].attenuation.y,
          lights[i].attenuation.z,
          material_shininess_factor
        );
      }
      diffuse += lighting.diffuse;
      specular += lighting.specular;
    }

  	vec3 combined_lighting = material_ambient_factor * ambient
                             + material_diffuse_factor * diffuse
                             + material_specular_factor * specular;

  	vec3 lighted_color = base_color() * min(combined_lighting, 1);
    // SNAP

    fragColor = vec4(lighted_color, 1);
    gl_FragDepth = depth;
}
