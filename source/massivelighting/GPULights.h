#pragma once

#include <glm/vec4.hpp>

/*mappings for light types
    fields		|		uni (type==0)					|		spot (type==2)								|		area  (type==4)		|

position		vec4(position.xyz,0)					vec4(position.xyz,2)								vec4(center.xyz,4)
color			vec4(color.rgb,1)						vec4(color.rgb,1)									vec4(color.rgb,1)
attenuation		vec4(constAtt,linearAtt,quadricAtt,0)	vec4(constAtt,linearAtt,quadricAtt,spotExponent)	vec4(constAtt,linearAtt,quadricAtt,width)
multiuse		not used								vec4(spotDirection.xyz,spotCosCutOf)				vec4(planeNormal.xyz,height)

*/

#define MAX_LIGHTS 256
#define MAX_LIGHT_INDICES 4096

typedef struct
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 attenuation;
    glm::vec4 multiuse;
} GPULight;

typedef struct
{
    glm::vec4 ambient_color;
    GPULight lights[MAX_LIGHTS];
    glm::uint number_of_lights;
} GPULights;