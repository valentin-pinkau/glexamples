#pragma once

#include <memory>
#include <vector>
#include <map>

#include <glm/vec3.hpp>

#include <globjects/base/ref_ptr.h>
#include <globjects/Framebuffer.h>
#include <globjects/Program.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/primitives/UniformGroup.h>

#define MAX_LIGHTS 16

namespace globjects
{
	class Texture;
}

namespace gloperate
{
    class AdaptiveGrid;
    class Icosahedron;
    class AbstractViewportCapability;
    class AbstractVirtualTimeCapability;
    class AbstractProjectionCapability;
    class AbstractCameraCapability;
    class AbstractTypedRenderTargetCapability;
	class PolygonalDrawable;
}

/*mappings for light types
	fields		|		uni (type==0)					|		spot (type==2)								|		area  (type==4)		|
position		vec4(position.xyz,0)					vec4(position.xyz,2)								vec4(center.xyz,4)
color			vec4(color.rgb,1)						vec4(color.rgb,1)									vec4(color.rgb,1)
attenuation		vec4(constAtt,linearAtt,quadricAtt,0)	vec4(constAtt,linearAtt,quadricAtt,spotExponent)	vec4(constAtt,linearAtt,quadricAtt,width)
multiuse		not used								vec4(spotDirection.xyz,spotCosCutOf)				vec4(planeNormal.xyz,height)

*/
struct Light
{
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 attenuation;
	glm::vec4 multiuse;
};

struct Lights
{
	glm::vec4 ambient_color;
	Light lights[MAX_LIGHTS];
	glm::uint number_of_lights;
};

class MassiveLightingRenderStage : public gloperate::AbstractStage
{
public:
    MassiveLightingRenderStage();
    virtual ~MassiveLightingRenderStage() = default;
    virtual void initialize() override;

public:
	gloperate::InputSlot<std::vector<std::unique_ptr<gloperate::PolygonalDrawable>>> drawables;
	gloperate::InputSlot<std::vector<globjects::ref_ptr<globjects::Texture>>> materials;
    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<gloperate::AbstractProjectionCapability *> projection;

    gloperate::Data<globjects::ref_ptr<globjects::Texture>> colorTexture;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> idTexture;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> normalTexture;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> geometryTexture;
    gloperate::Data<globjects::ref_ptr<globjects::Texture>> depthBufferTexture;

protected:
    virtual void process() override;
    void setupGLState();
    void loadShader();
    void setupFbo();
    void setupUniforms();
    void resizeFbos(int width, int height);
    void render();


protected:
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    globjects::ref_ptr<globjects::Program> m_program;
    gloperate::UniformGroup m_uniforms;
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
	globjects::ref_ptr<globjects::Buffer> m_lights;
};


