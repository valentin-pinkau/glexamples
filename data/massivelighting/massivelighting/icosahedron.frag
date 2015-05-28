#version 330


uniform sampler2D material;


in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_texCoords;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out float fragID;
layout(location = 2) out vec4 fragNormal;

vec3 base_color()
{
	return texture(material, v_texCoords.xy).rgb;
}

void main()
{

	fragColor = vec4(base_color(), 1.0);
	fragID = 0;
	fragNormal = vec4(normalize(v_normal), 1.0);
}
