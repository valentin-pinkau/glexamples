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
uniform sampler2D material;
const vec4 base_color = vec4(0.5, 1, 0, 1);
const float ambient_factor = 0.4;
const float diffuse_factor = 0.4;
const float specular_factor = 0.4;

const float ambient_factor = 0.2;
const float diffuse_factor = 0.3;
const float specular_factor = 0.6;

in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_texCoords;

out vec4 fragColor;

vec3 base_color()
{
	return texture2D(material, v_texCoords.xy).rgb;
}

void main()
{
	float light_factor = ambient_factor;

	vec3 normal = normalize(v_normal);
	vec3 view_direction = normalize(eye - v_vertex);


	for (uint i = 0u; i < number_of_lights; ++i)
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
  
vec4 GetPointColor(Light light, vec3 pos, vec3 viewDir, vec3 normal)
{
  vec3 n,halfV;
  vec3 lightDir;
  float NdotL,NdotHV;
  vec4 color = base_color; //globalAmbient
  float att,spotEffect;
  //per light parameters
  float spotCosCutoff = 0.5;
  float spotExponent = 10;
  vec3 spotDirection = vec3(-2,0,-1);
  float constantAttenuation = 0.5;
  float linearAttenuation = 0.2;
  float quadraticAttenuation = 0.2;
  vec4  diffuse_color = vec4(1,0,0,0);
  vec4  specular_color = vec4(0,1,0,0);
  //per Material parameters
  float materialSpecular = 0.6;
  float materialShininess = 0.6;
  
  
   
  /* a fragment shader can't write a verying variable, hence we need
  a new variable to store the normalized interpolated normal */
  n = normalize(normal);

  // Compute the ligt direction
  lightDir = vec3(light.position.xyz-pos);

  /* compute the distance to the light source to a varying variable*/
  dist = length(lightDir);
  lightDir = normalize(lightDir);

  /* compute the dot product between normal and ldir */
  NdotL = max(dot(n,lightDir),0.0);
  if (NdotL > 0.0) {
      spotEffect = dot(normalize(spotDirection), -lightDir);
      if (spotEffect > spotCosCutoff) {
          spotEffect = pow(spotEffect, spotExponent);
          att = spotEffect / (constantAttenuation +
                  linearAttenuation * dist +
                  quadraticAttenuation * dist * dist);
               
          color += att * (diffuse_color * NdotL + ambient);
          
          halfV = normalize((viewDir + lightDir) / 2);
          NdotHV = max(dot(n,halfV),0.0);
          color += att * materialSpecular * specular_factor * pow(NdotHV,materialShininess);
      }
  }
  return color;
}
