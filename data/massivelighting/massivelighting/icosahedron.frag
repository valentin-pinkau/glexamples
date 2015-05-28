#version 150 core


uniform sampler2D material;


in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_texCoords;

out vec4 fragColor;

vec3 base_color()
{
	return texture(material, v_texCoords.xy).rgb;
}

void main()
{

	fragColor = vec4(base_color(), 1.0);
}