#pragma once

#include <glm/vec2.hpp>

#include <globjects/base/ref_ptr.h>

#include <globjects/Framebuffer.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Texture.h>

#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/primitives/UniformGroup.h>


namespace gloperate
{

class AbstractViewportCapability;
class AbstractTargetFramebufferCapability;

}


class PostprocessingStage : public gloperate::AbstractStage
{
public:
    PostprocessingStage();

    virtual void initialize() override;
public:
    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> colorTexture;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> depthTexture;
    gloperate::InputSlot<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
protected:
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Shader> m_fragShader;
    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
    gloperate::UniformGroup m_uniforms;

    virtual void process() override;
};

