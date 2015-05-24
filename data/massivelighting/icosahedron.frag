#version 150 core

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

uniform vec3 eye;
uniform sampler2D material;

const float material_ambient_factor = 0.2;
const float material_diffuse_factor = 0.3;
const float material_specular_factor = 0.6;
const float material_shininess_factor = 16;

in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_texCoords;

out vec4 fragColor;

vec3 base_color()
{
	return texture(material, v_texCoords.xy).rgb;
}

vec3 projectOnPlane(in vec3 point, in vec3 pointOnPlane, in vec3 planeNormal) {
	float distance = dot( planeNormal, point-pointOnPlane );
	return point - distance * planeNormal;
}
float sideOfPlane( in vec3 point, in vec3 pointOnPlane, in vec3 planeNormal ) {
	return sign( dot( point - pointOnPlane, planeNormal ) );
}
vec3 linePlaneIntersect( in vec3 pointOnLine, in vec3 lineDirection, in vec3 pointOnPlane, in vec3 planeNormal ) {
	return pointOnLine + lineDirection * ( dot( planeNormal, pointOnPlane - pointOnLine ) / dot( planeNormal, lineDirection ) );
}

float attenuation(
float light_attenuation_factor,
float light_constant_attenuation,
float light_linear_attenuation,
float light_quadratic_attenuation,
float light_distance) {
  return light_attenuation_factor / (light_constant_attenuation +
                  light_linear_attenuation * light_distance +
                  light_quadratic_attenuation * light_distance * light_distance);
}

vec2 unidirectionalLighting(
vec3 v_eye,
vec3 v_vertex,
vec3 v_normal,
vec3 light_position,
vec3 light_color,
float light_constant_attenuation,
float light_linear_attenuation,
float light_quadratic_attenuation,
float material_shininess_factor
) {
    vec2 result = vec2(0,0);
    vec3 view_direction = normalize(v_eye - v_vertex);
    vec3 light_direction = light_position - v_vertex;
    float light_distance = length(light_direction);
		float NdotL = max(dot(normalize(light_direction), v_normal), 0.0);
    //only shade front face
    if (NdotL > 0.0) {
      float att = attenuation(1.0, light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_distance);
                     
      vec3 half_direction = normalize(light_direction + view_direction);
      float specular_angle = max(dot(half_direction, v_normal), 0.0);
      float specular_intensity = pow(specular_angle, material_shininess_factor);
      
      result.r += att * light_color * NdotL;
      result.g += att * specular_intensity;
    }
    return result;
}

vec2 spotLighting(
vec3 v_eye,
vec3 v_vertex,
vec3 v_normal,
vec3 light_position,
vec3 light_color,
float light_constant_attenuation,
float light_linear_attenuation,
float light_quadratic_attenuation,
float spot_cos_cutoff,
float spot_exponent,
vec3  spot_direction,
float material_shininess_factor
) {    
  vec2 result = vec2(0,0);
  vec3 view_direction = normalize(v_eye - v_vertex);
  vec3 light_direction = light_position - v_vertex;
  float light_distance = length(light_direction);
  float NdotL = max(dot(normalize(light_direction), v_normal), 0.0); 
  //only shade front face
  if (NdotL > 0.0) {
    float light_attenuation_factor = dot(normalize(spot_direction), normalize(-light_direction));
    //if fragment is outside spot cone
    if (light_attenuation_factor > 0 || abs(light_attenuation_factor) < spot_cos_cutoff) { return result; }

    light_attenuation_factor = pow(light_attenuation_factor, spot_exponent);                          
    float att = attenuation(light_attenuation_factor, light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_distance);
                      
    vec3 half_direction = normalize(light_direction + view_direction);
    float specular_angle = max(dot(half_direction, v_normal), 0.0);
    float specular_intensity = pow(specular_angle, material_shininess_factor);
    result.r += att * light_color * NdotL;
    result.g += att * specular_intensity;
  }
  return result;
}

vec2 areaLighting(
vec3 v_eye,
vec3 v_vertex,
vec3 v_normal,
vec3 light_position,
vec3 light_normal,
vec3 light_right,
vec3 light_up,
float light_width,
float light_height,
vec3 light_color,
float light_constant_attenuation,
float light_linear_attenuation,
float light_quadratic_attenuation,
float material_shininess_factor
) {
  vec2 result = vec2(0,0);
  vec3 proj = projectOnPlane( v_vertex.xyz, light_position, light_normal );
  vec3 dir = proj - light_position;
  vec3 view_direction = normalize(v_eye - v_vertex);

  vec2 diagonal = vec2( dot( dir, light_right ), dot( dir, light_up ) );
  vec2 nearest2D = vec2( clamp( diagonal.x, -light_width, light_width ), clamp( diagonal.y, -light_height, light_height ) );
  vec3 nearestPointInside = vec3( light_position ) + ( light_right * nearest2D.x + light_up * nearest2D.y );

  vec3 lightDir = normalize( nearestPointInside - v_vertex.xyz );
  float NdotL = max( dot( light_normal, -lightDir ), 0.0 );
  float NdotL2 = max( dot( v_normal, lightDir ), 0.0 );

  //if ( NdotL2 * NdotL > 0.0 && sideOfPlane( v_vertex.xyz, light_position, light_normal ) ) {
  if ( NdotL2 * NdotL > 0.0 ) {

    // specular
    vec3 R = reflect( normalize( -view_direction ), v_normal );
    vec3 E = linePlaneIntersect( v_vertex.xyz, R, vec3( light_position ), light_normal );
    float light_distance = distance( v_vertex.xyz, nearestPointInside );
    float att = attenuation(1.0, light_constant_attenuation, light_linear_attenuation, light_quadratic_attenuation, light_distance);

    float specAngle = dot( R, light_normal );

    if ( specAngle > 0.0 ) {
      vec3 dirSpec = E - vec3( light_position );
      vec2 dirSpec2D = vec2( dot( dirSpec, light_right ), dot( dirSpec, light_up ) );
      vec2 nearestSpec2D = vec2( clamp( dirSpec2D.x, -light_width, light_width ), clamp( dirSpec2D.y, -light_height, light_height ) );
      float specFactor = 1.0 - clamp( length( nearestSpec2D - dirSpec2D ) * 0.05 * material_shininess_factor, 0.0, 1.0 );
      result.g = att * specFactor * specAngle;
    }
    result.r += att * light_color * sqrt( NdotL * NdotL2 );
  }
  return result;
}

void main()
{
	float light_factor = material_ambient_factor;

	vec3 normal = normalize(v_normal);

	vec3 ambient = ambient_color.rgb;
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	for (uint i = 0u; i < number_of_lights; i++)
	{
    vec2 lighting;
    if (lights[i].position.w > 3.0) {
        //lighting = areaLighting();
      }
      //spot light
      else if (lights[i].position.w > 1.0) {
        lighting = spotLighting(
          eye,
          v_vertex,
          normal,
          lights[i].position.xyz,
          lights[i].color.rgb,
          lights[i].attenuation.x,
          lights[i].attenuation.y,
          lights[i].attenuation.z,
          lights[i].multiuse.w,
          lights[i].attenuation.w,
          lights[i].multiuse.xyz,
          material_shininess_factor
        );
      }
      //uni
      else {
        lighting = unidirectionalLighting(
          eye,
          v_vertex,
          normal,
          lights[i].position.xyz,
          lights[i].color.rgb,
          lights[i].attenuation.x,
          lights[i].attenuation.y,
          lights[i].attenuation.z,
          material_shininess_factor
        );
      }
      diffuse += lighting.r;
      specular += lighting.g;
	}

	vec3 combined_lighting = material_ambient_factor * ambient
                         + 
                         material_diffuse_factor * diffuse
                         +
												 material_specular_factor * specular
                         ;

	vec3 lighted_color = base_color() * min(combined_lighting, 1);
	fragColor = vec4(lighted_color, 1.0);
}