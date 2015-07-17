#include "BasicLightingPostprocessingStage.h"

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>
#include <globjects/Shader.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>



BasicLightingPostprocessingStage::BasicLightingPostprocessingStage()
: AbstractStage("Postprocessing")

{
    alwaysProcess(true);

    addInput("viewport", viewport);
    addInput("colorTexture", colorTexture);
    addInput("depthTexture", depthTexture);
    addOptionalInput("targetFBO", targetFBO);
}

void BasicLightingPostprocessingStage::initialize()
{
    m_program = new globjects::Program{};

    m_program->attach(
		globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/massivelighting/shaders/basiclighting/postprocessing.vert", { "data/massivelighting/shaders/common" }),
		globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/massivelighting/shaders/basiclighting/postprocessing.frag", { "data/massivelighting/shaders/common" })
    );

    m_uniforms.addUniform(new globjects::Uniform<int>("colorTexture", 0));
    m_uniforms.addUniform(new globjects::Uniform<int>("depthTexture", 1));
    m_uniforms.addToProgram(m_program);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_program);
}

void BasicLightingPostprocessingStage::process()
{
    if(!m_fbo)
    {
        m_fbo = targetFBO.isConnected() && targetFBO.data()->framebuffer() ? targetFBO.data()->framebuffer() : globjects::Framebuffer::defaultFBO();
    }
    m_fbo->bind();
    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

    colorTexture.data()->bindActive(gl::GL_TEXTURE0);
    depthTexture.data()->bindActive(gl::GL_TEXTURE1);

    m_screenAlignedQuad->draw();

    depthTexture.data()->unbindActive(gl::GL_TEXTURE1);
    colorTexture.data()->unbindActive(gl::GL_TEXTURE0);
    m_fbo->unbind();
}

