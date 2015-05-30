#include "MassiveLightingPostprocessingStage.h"

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>
#include <globjects/Shader.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>


MassiveLightingPostprocessingStage::MassiveLightingPostprocessingStage()
: AbstractStage("Postprocessing")

{
    alwaysProcess(true);

    addInput("viewport", viewport);
    addInput("lights Buffer", lightsBuffer);
	addInput("colorTexture", colorTexture);
	addInput("normalTexture", normalTexture);
    addInput("depthTexture", depthTexture);
    addOptionalInput("targetFBO", targetFBO);
}

void MassiveLightingPostprocessingStage::initialize()
{
    m_program = new globjects::Program{};

    m_program->attach(
        globjects::Shader::fromFile(gl::GL_VERTEX_SHADER, "data/massivelighting/massivelighting/postprocessing.vert"),
        globjects::Shader::fromFile(gl::GL_FRAGMENT_SHADER, "data/massivelighting/massivelighting/postprocessing.frag")
    );

	m_uniforms.addUniform(new globjects::Uniform<glm::mat4>("transformInverted", glm::mat4()));
	m_uniforms.addUniform(new globjects::Uniform<glm::vec3>("eye", glm::vec3()));
	m_uniforms.addUniform(new globjects::Uniform<int>("colorTexture", 0));
	m_uniforms.addUniform(new globjects::Uniform<int>("normalTexture", 1));
	m_uniforms.addUniform(new globjects::Uniform<int>("depthTexture", 2));
    m_uniforms.addToProgram(m_program);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_program);
}

void MassiveLightingPostprocessingStage::process()
{
	if (camera.hasChanged() || projection.hasChanged())
	{
		const auto transformInverted = camera.data()->viewInverted() * projection.data()->projectionInverted();
		const auto eye = camera.data()->eye();
		m_uniforms.uniform<glm::mat4>("transformInverted")->set(transformInverted);
		m_uniforms.uniform<glm::vec3>("eye")->set(eye);
	}

    if(!m_fbo)
    {
        m_fbo = targetFBO.isConnected() && targetFBO.data()->framebuffer() ? targetFBO.data()->framebuffer() : globjects::Framebuffer::defaultFBO();
    }
    m_fbo->bind();

    if(lightsBuffer.hasChanged() && lightsBuffer.data())
    {
        auto uniformBlock = m_program->uniformBlock("Lights");
        lightsBuffer.data()->bindBase(gl::GL_UNIFORM_BUFFER, 0);
        uniformBlock->setBinding(0);
    }

    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

    colorTexture.data()->bindActive(gl::GL_TEXTURE0);
    normalTexture.data()->bindActive(gl::GL_TEXTURE1);
	depthTexture.data()->bindActive(gl::GL_TEXTURE2);

    m_screenAlignedQuad->draw();

    depthTexture.data()->unbindActive(gl::GL_TEXTURE2);
	normalTexture.data()->unbindActive(gl::GL_TEXTURE1);
    colorTexture.data()->unbindActive(gl::GL_TEXTURE0);
    m_fbo->unbind();
}

