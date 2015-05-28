#version 140
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform mat4 transform;
uniform vec3 eye;

layout (location = 0) out vec4 fragColor;

in vec2 v_uv;

void main()
{
    fragColor = texture(colorTexture, v_uv);
    gl_FragDepth = texture(depthTexture, v_uv).r;
}
