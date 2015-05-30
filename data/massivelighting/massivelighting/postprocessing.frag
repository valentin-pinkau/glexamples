#version 140
#extension GL_ARB_explicit_attrib_location : require

const float material_ambient_factor = 0.2;
const float material_diffuse_factor = 0.5;
const float material_specular_factor = 0.6;
const float material_shininess_factor = 16;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform mat4 transformInverted;
uniform vec3 eye;

in vec2 v_uv;
in vec2 v_screenCoordinates;

layout (location = 0) out vec4 fragColor;

#define MAX_LIGHTS 16

struct Light {
	vec4 position;
	vec4 color;
  vec4 attenuation;
  vec4 multiuse;
};

layout (std140) uniform Lights
{
		vec4 ambient_color;
		Light lights[MAX_LIGHTS];
		uint number_of_lights;
};

void main()
{
    vec3 normal = texture(normalTexture, v_uv).xyz;
    float depth = texture(depthTexture, v_uv).r;
    // depth is in [0;1] whereas NDCs are in [-1;1]^3
    vec4 almostWorldCoordinates = transformInverted * vec4(v_screenCoordinates, depth * 2.0 - 1.0, 1);
    vec4 worldCoordinates = vec4(almostWorldCoordinates.xyz / almostWorldCoordinates.w, 1);

    // SNIP
    vec3 view_direction = normalize(eye - worldCoordinates.xyz);

  	vec3 ambient = ambient_color.rgb;
  	vec3 diffuse = vec3(0);
  	vec3 specular = vec3(0);

  	for (uint i = 0u; i < number_of_lights; ++i)
  	{
      float light_constant_attenuation = lights[i].attenuation.x;
      float light_linear_attenuation = lights[i].attenuation.y;
      float light_quadratic_attenuation = lights[i].attenuation.z;

      vec3 light_direction = lights[i].position.xyz - worldCoordinates.xyz;
      float light_distance = length(light_direction);
  		float NdotL = max(dot(normalize(light_direction), normal), 0.0);
      float light_attenuation_factor = 1.0;
      //only shade front face
      if (NdotL > 0.0) {
        //area
        if (lights[i].position.w > 3.0) {
          continue;
        }
        //spot light
        else if (lights[i].position.w > 2.0) {
          float spot_cos_cutoff = lights[i].multiuse.w;
          float spot_exponent = lights[i].attenuation.w;
          vec3  spot_direction = lights[i].multiuse.xyz;

          float light_attenuation_factor = dot(normalize(spot_direction), normalize(-light_direction));
          //if fragment is outside spot cone
          if (light_attenuation_factor > 0 || abs(light_attenuation_factor) < spot_cos_cutoff) { continue; }
          light_attenuation_factor = pow(light_attenuation_factor, spot_exponent);
        }
        //uni
        else {

        }

        float att = light_attenuation_factor / (light_constant_attenuation +
                    light_linear_attenuation * light_distance +
                    light_quadratic_attenuation * light_distance * light_distance);

        diffuse += att * lights[i].color.xyz * NdotL;

        vec3 half_direction = normalize((light_direction + view_direction) /2);
        float specular_angle = max(dot(half_direction, normal), 0.0);
        float specular_intensity = pow(specular_angle, material_shininess_factor);
        specular += att * specular_intensity;
      }
  	}

  	vec3 combined_lighting = material_ambient_factor * ambient
                           + material_diffuse_factor * diffuse
                           + material_specular_factor * specular;

  	vec3 lighted_color = texture(colorTexture, v_uv).rgb * min(combined_lighting, 1);
    // SNAP

    fragColor = vec4(lighted_color, 1);
    gl_FragDepth = depth;
}
