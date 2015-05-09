#version 150 core

#define MAX_LIGHTS 16

struct Light {
	vec4 position;
	vec4 color;
};

layout (std140) uniform Lights
{
		vec4 ambient_color;
		Light lights[MAX_LIGHTS];
		uint number_of_lights;
};

uniform vec3 eye;

const float ambient_factor = 0.2;
const float diffuse_factor = 0.3;
const float specular_factor = 0.6;

in vec3 v_normal;
in vec3 v_vertex;

out vec4 fragColor;

vec3 base_color()
{
	// TODO: Access texture here, or use per-vertex color attribute
	return vec3(0.8, 0.8, 0.8);
}

void main()
{
	float light_factor = ambient_factor;

	vec3 normal = normalize(v_normal);
	vec3 view_direction = normalize(eye - v_vertex);

	vec3 ambient = ambient_color.rgb;
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	for (int i = 0; i < number_of_lights; ++i)
	{
		vec3 light_direction = lights[i].position.xyz - v_vertex;
		float lambertian = max(dot(normalize(light_direction), normal), 0.0);

		diffuse += lights[i].color.xyz * lambertian;

		vec3 half_direction = normalize(light_direction + view_direction);
		float specular_angle = max(dot(half_direction, normal), 0.0);
		float specular_intensity = pow(specular_angle, 16.0);

		specular += lights[i].color.xyz * specular_intensity;
	}

	vec3 combined_lighting = ambient_factor * ambient
	  										 + diffuse_factor * diffuse
												 + specular_factor * specular;

	vec3 lighted_color = base_color() * min(combined_lighting, 1);
	fragColor = vec4(lighted_color, 1.0);
}
