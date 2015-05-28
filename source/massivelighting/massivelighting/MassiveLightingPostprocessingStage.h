#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/pipeline/Data.h>

#include <gloperate/primitives/UniformGroup.h>

namespace globjects
{

class Framebuffer;
class Program;
class Texture;

}

namespace gloperate
{

class AbstractViewportCapability;
class AbstractProjectionCapability;
class AbstractCameraCapability;
class AbstractTargetFramebufferCapability;
class ScreenAlignedQuad;

}

class MassiveLightingPostprocessingStage : public gloperate::AbstractStage
{
public:
    MassiveLightingPostprocessingStage();

    virtual void initialize() override;
public:
	gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
	gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
	gloperate::InputSlot<gloperate::AbstractProjectionCapability *> projection;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> colorTexture;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> depthTexture;
	gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> normalTexture;
    gloperate::InputSlot<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
protected:
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
    gloperate::UniformGroup m_uniforms;

    virtual void process() override;
};

