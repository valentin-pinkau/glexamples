#version 140
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require
#include <phong.glsl>

#define MAX_LIGHTS 256

const float material_ambient_factor = 0.2;
const float material_diffuse_factor = 0.5;
const float material_specular_factor = 0.6;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform isampler3D clusterTexture;
uniform isampler1D lightIndicesTexture;

uniform mat4 transformInverted;
uniform vec3 eye;

in vec2 v_uv;
in vec2 v_screenCoordinates;

layout (location = 0) out vec4 fragColor;

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

		vec3 tcolor;

		ivec2 lookup = texture(clusterTexture, vec3(v_uv, depth)).xy;
  	for (int li = 0; li < lookup.y; ++li)
    {
			int i = texelFetch(lightIndicesTexture, lookup.x + li, 0).x;

			LightingInfo lighting = computeLighting(eye, worldCoordinates.xyz, normal, lights[i]);
			tcolor = lights[i].color.rgb;
      diffuse += lighting.diffuse;

      specular += lighting.specular;
    }
		if (lookup.y == 2) tcolor = vec3(1);

  	vec3 combined_lighting = material_ambient_factor * ambient
                             + material_diffuse_factor * diffuse
                             + material_specular_factor * specular;

  	vec3 lighted_color = base_color() * min(combined_lighting, 1);

		if (v_screenCoordinates.x < 0)
    	fragColor = vec4(lighted_color, 1);
		else
			fragColor = vec4(vec3(lookup.y) / 32.f, 1);
			//fragColor = vec4(tcolor, 1);
    gl_FragDepth = depth;
}
