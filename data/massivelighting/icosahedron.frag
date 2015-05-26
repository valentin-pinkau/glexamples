#version 150 core

#define MAX_LIGHTS 256

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

uniform vec3 eye;
uniform sampler2D material;

const float material_ambient_factor = 0.2;
const float material_diffuse_factor = 0.5;
const float material_specular_factor = 0.6;
const float material_shininess_factor = 16;

in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_texCoords;

out vec4 fragColor;

vec4 base_color()
{
	return texture(material, v_texCoords.xy);
}

void main()
{
	float light_factor = material_ambient_factor;

	vec3 normal = normalize(v_normal);
	vec3 view_direction = normalize(eye - v_vertex);

	vec3 ambient = ambient_color.rgb;
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	for (uint i = 0u; i < number_of_lights; i++)
	{
    float light_constant_attenuation = lights[i].attenuation.x;
    float light_linear_attenuation = lights[i].attenuation.y;
    float light_quadratic_attenuation = lights[i].attenuation.z;

    vec3 light_direction = lights[i].position.xyz - v_vertex;
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
      else if (lights[i].position.w > 1.0) {

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
      specular += att * specular_intensity * lights[i].color.xyz;
    }
	}

	vec3 combined_lighting = material_ambient_factor * ambient
                         +
                         material_diffuse_factor * diffuse
                         +
												 material_specular_factor * specular
                         ;

  vec4 temp = base_color();
	if (temp.w == 0)
		discard;
	vec3 lighted_color = temp.rgb * min(combined_lighting, 1);
	fragColor = vec4(lighted_color, 1);
}
