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

using namespace gl;

MassiveLightingPostprocessingStage::MassiveLightingPostprocessingStage()
	: AbstractStage("Postprocessing") {
    addInput("viewport", viewport);
    addInput("lights Buffer", lightsBuffer);
	addInput("colorTexture", colorTexture);
	addInput("normalTexture", normalTexture);
    addInput("depthTexture", depthTexture);
	addInput("clusterTexture", clusterTexture);
	addInput("lightIndicesTexture", lightIndicesTexture);
	addInput("enableDebugOutput", enableDebugOutput);
    addOptionalInput("targetFBO", targetFBO);
	
	alwaysProcess(true);
}

void MassiveLightingPostprocessingStage::initialize()
{
    m_program = new globjects::Program{};
    m_program->setName("PostprocessingProgramm");
    m_program->attach(
        globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/massivelighting/shaders/massivelighting/postprocessing.vert"),
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/massivelighting/shaders/massivelighting/postprocessing.frag")
    );

	m_uniforms.addUniform(new globjects::Uniform<glm::mat4>("transformInverted", glm::mat4()));
	m_uniforms.addUniform(new globjects::Uniform<glm::vec3>("eye", glm::vec3()));
	m_uniforms.addUniform(new globjects::Uniform<glm::int32>("enableDebugOutput", 0));
	m_uniforms.addUniform(new globjects::Uniform<int>("colorTexture", 0));
	m_uniforms.addUniform(new globjects::Uniform<int>("normalTexture", 1));
	m_uniforms.addUniform(new globjects::Uniform<int>("depthTexture", 2));
	m_uniforms.addUniform(new globjects::Uniform<int>("clusterTexture", 3));
	m_uniforms.addUniform(new globjects::Uniform<int>("lightIndicesTexture", 4));
    m_uniforms.addToProgram(m_program);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_program);
}

void MassiveLightingPostprocessingStage::process()
{
	if (enableDebugOutput.hasChanged())
	{
		m_uniforms.uniform<glm::int32>("enableDebugOutput")->set(enableDebugOutput.data());
	}

	if (camera.hasChanged() || projection.hasChanged())
	{
		const auto transformInverted = camera.data()->viewInverted() * projection.data()->projectionInverted();
		const auto eye = camera.data()->eye();
		m_uniforms.uniform<glm::mat4>("transformInverted")->set(transformInverted);
		m_uniforms.uniform<glm::vec3>("eye")->set(eye);
	}

    if (!m_fbo)
    {
        m_fbo = targetFBO.isConnected() && targetFBO.data()->framebuffer() ? targetFBO.data()->framebuffer() : globjects::Framebuffer::defaultFBO();
    }
    m_fbo->bind();

    if (lightsBuffer.hasChanged() && lightsBuffer.data())
    {
        auto uniformBlock = m_program->uniformBlock("Lights");
        lightsBuffer.data()->bindBase(GL_UNIFORM_BUFFER, 0);
        uniformBlock->setBinding(0);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    colorTexture.data()->bindActive(GL_TEXTURE0);
    normalTexture.data()->bindActive(GL_TEXTURE1);
	depthTexture.data()->bindActive(GL_TEXTURE2);
	clusterTexture.data()->bindActive(GL_TEXTURE3);
	lightIndicesTexture.data()->bindActive(GL_TEXTURE4);

    m_screenAlignedQuad->draw();

	lightIndicesTexture.data()->unbindActive(GL_TEXTURE4);
	clusterTexture.data()->unbindActive(GL_TEXTURE3);
    depthTexture.data()->unbindActive(GL_TEXTURE2);
	normalTexture.data()->unbindActive(GL_TEXTURE1);
    colorTexture.data()->unbindActive(GL_TEXTURE0);
    m_fbo->unbind();
}

