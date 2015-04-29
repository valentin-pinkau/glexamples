#include "BlitStage.h"

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>

#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>



BlitStage::BlitStage()
: AbstractStage("Blitting")
{
    alwaysProcess(true);

    addInput("viewport", viewport);
    addInput("blitTexture", texture);
    addOptionalInput("targetFBO", targetFBO);
}

void BlitStage::initialize()
{
    m_fbo = new globjects::Framebuffer;
    m_fbo->setName("Blit FBO");

    m_fbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, texture.data());
}

void BlitStage::process()
{
    std::array<gl::GLint, 4> srcRect = {{ viewport.data()->x(), viewport.data()->y(), viewport.data()->width(), viewport.data()->height() }};
    std::array<gl::GLint, 4> destRect = {{ viewport.data()->x(), viewport.data()->y(), viewport.data()->width(), viewport.data()->height() }};

    globjects::Framebuffer* destFBO = targetFBO.isConnected() && targetFBO.data()->framebuffer() ? targetFBO.data()->framebuffer() : globjects::Framebuffer::defaultFBO();

    m_fbo->blit(gl::GL_COLOR_ATTACHMENT0, srcRect, destFBO, destFBO->id() == 0 ? gl::GL_BACK_LEFT : gl::GL_COLOR_ATTACHMENT0, destRect, gl::GL_COLOR_BUFFER_BIT, gl::GL_LINEAR);
}

