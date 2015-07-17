#version 150 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require
#include <phong.glsl>

#define MAX_LIGHTS 256

uniform vec3 eye;
uniform sampler2D material;

const float material_ambient_factor = 0.2;
const float material_diffuse_factor = 0.5;
const float material_specular_factor = 0.6;

in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_texCoords;

layout (std140) uniform Lights
{
		vec4 ambient_color;
		Light lights[MAX_LIGHTS];
		uint number_of_lights;
};

out vec4 fragColor;

vec3 base_color()
{
	return texture(material, v_texCoords.xy).rgb;
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
    LightingInfo lighting = computeLighting(eye, v_vertex, normal, lights[i]);
		diffuse += lighting.diffuse;
		specular += lighting.specular;
	}

	vec3 combined_lighting = material_ambient_factor * ambient
													+ material_diffuse_factor * diffuse
													+ material_specular_factor * specular;

	vec3 lighted_color = base_color() * min(combined_lighting, 1);
	fragColor = vec4(lighted_color, 1.0);
}
