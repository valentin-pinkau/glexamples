#pragma once

#include <memory>
#include <vector>
#include <map>

#include <glm/vec4.hpp>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/primitives/Light.h>

#include <reflectionzeug/FilePath.h>

namespace globjects
{
	class Texture;
}

namespace gloperate
{
	class ResourceManager;
}

/*mappings for light types
    fields		|		uni (type==0)					|		spot (type==2)								|		area  (type==4)		|

position		vec4(position.xyz,0)					vec4(position.xyz,2)								vec4(center.xyz,4)
color			vec4(color.rgb,1)						vec4(color.rgb,1)									vec4(color.rgb,1)
attenuation		vec4(constAtt,linearAtt,quadricAtt,0)	vec4(constAtt,linearAtt,quadricAtt,spotExponent)	vec4(constAtt,linearAtt,quadricAtt,width)
multiuse		not used								vec4(spotDirection.xyz,spotCosCutOf)				vec4(planeNormal.xyz,height)

*/

#define MAX_LIGHTS 16

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
    GPULight lights[MAX_LIGHTS];
    glm::uint number_of_lights;
};


class GeometryStage : public gloperate::AbstractStage
{
public:
	GeometryStage();
    virtual ~GeometryStage() = default;
    virtual void initialize() override;

public:
    gloperate::InputSlot<reflectionzeug::FilePath> sceneFilePath;
	gloperate::InputSlot<gloperate::ResourceManager *> resourceManager;
    gloperate::Data<std::vector<std::unique_ptr<gloperate::PolygonalDrawable>>> drawables;
	gloperate::Data<std::vector<globjects::ref_ptr<globjects::Texture>>> materials;
    gloperate::Data<globjects::ref_ptr<globjects::Buffer>> lightsBuffer;

protected:
    virtual void process() override;
	void reloadScene();

};


