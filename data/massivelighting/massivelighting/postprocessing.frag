#version 140
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

layout (location = 0) out vec4 fragColor;

in vec2 v_uv;

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
    fragColor = texture(colorTexture, v_uv);
    gl_FragDepth = texture(depthTexture, v_uv).r;
}
