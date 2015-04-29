#pragma once

#include <globjects/base/ref_ptr.h>
#include <globjects/Framebuffer.h>
#include <globjects/Program.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/primitives/UniformGroup.h>


namespace gloperate
{
    class AdaptiveGrid;
    class Icosahedron;
    class AbstractViewportCapability;
    class AbstractVirtualTimeCapability;
    class AbstractProjectionCapability;
    class AbstractCameraCapability;
    class AbstractTypedRenderTargetCapability;
}

class MassiveLightingRenderStage : public gloperate::AbstractStage
{
public:
    MassiveLightingRenderStage();
    virtual ~MassiveLightingRenderStage() = default;
    virtual void initialize() override;

public:

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
    globjects::ref_ptr<gloperate::Icosahedron> m_icosahedron;
    globjects::ref_ptr<globjects::Program> m_program;
    gloperate::UniformGroup m_uniforms;
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
};


