#pragma once

#include <glm/vec4.hpp>

/*mappings for light types
    fields		|		uni (type==2)					|		spot (type==3)								|		area  (type==5)		|

position		vec4(position.xyz,2)					vec4(position.xyz,3)								vec4(center.xyz,5)
color			vec4(color.rgb,1)						vec4(color.rgb,1)									vec4(color.rgb,1)
attenuation		vec4(constAtt,linearAtt,quadricAtt,0)	vec4(constAtt,linearAtt,quadricAtt,spotExponent)	vec4(constAtt,linearAtt,quadricAtt,width)
multiuse		not used								vec4(spotDirection.xyz,spotCosCutOf)				vec4(planeNormal.xyz,height)

*/

#define MAX_LIGHTS 1024

struct GPULight
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 attenuation;
    glm::vec4 multiuse;
};

struct GPULights
{
    glm::vec4 ambient_color;
    glm::uint number_of_lights;
    glm::uint padding1;
    glm::uint padding2;
    glm::uint padding3;
    GPULight lights[MAX_LIGHTS];

};
