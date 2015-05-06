#version 150 core

#define MAX_LIGHTS 16

struct Light {
	vec4 position;
	vec4 color;
};

layout (std140) uniform Lights
{
		Light lights[MAX_LIGHTS];
		uint number_of_lights;
};

const vec3 base_color = vec3(0.5, 1, 0);
const float ambient_factor = 0.4;
const float diffuse_factor = 0.4;

in vec3 v_normal;
in vec3 v_vertex;

out vec4 fragColor;

void main()
{
	float light_factor = ambient_factor;
	vec3 normal = normalize(v_normal);

	for (int i = 0; i < number_of_lights; ++i)
	{
		vec3 light_direction = lights[i].position.xyz - v_vertex;

		float lambertTerm = max(0.0, dot(normalize(light_direction), normal));

		light_factor += lambertTerm * diffuse_factor;
	}

	vec3 lighted_color = clamp(0, 1, light_factor) * base_color;
	fragColor = vec4(lighted_color, 1.0);
}
