#pragma once

#include <glm/vec2.hpp>

#include <globjects/base/ref_ptr.h>

#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>

#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/pipeline/Data.h>


namespace gloperate
{

class AbstractViewportCapability;
class AbstractTargetFramebufferCapability;

}


class BlitStage : public gloperate::AbstractStage
{
public:
    BlitStage();

    virtual void initialize() override;
public:
    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> texture;
    gloperate::InputSlot<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
protected:
    globjects::ref_ptr<globjects::Framebuffer> m_fbo;

    virtual void process() override;
};

