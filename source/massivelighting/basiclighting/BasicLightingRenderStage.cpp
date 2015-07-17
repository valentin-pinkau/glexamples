#include "BasicLightingRenderStage.h"

#include <glbinding/gl/bitfield.h>
#include <glbinding/callbacks.h>

#include <globjects/base/File.h>
#include <globjects/DebugMessage.h>

#include <globjects/Shader.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/globjects.h>

#include <gloperate/primitives/UniformGroup.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractVirtualTimeCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/Camera.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/primitives/VertexDrawable.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/primitives/Light.h>

using namespace gl;
using namespace globjects;


BasicLightingRenderStage::BasicLightingRenderStage()
:   AbstractStage("BasicLightingRenderStage")
{
    addInput("drawables", drawables);
	addInput("materials", materials);
    addInput("lights Buffer", lightsBuffer);
    addInput("viewport", viewport);
    addInput("camera", camera);
    addInput("projection", projection);

    addOutput("colorTexture", colorTexture);
    addOutput("idTexture", idTexture);
    addOutput("normalTexture", normalTexture);
    addOutput("geometryTexture", geometryTexture);
    addOutput("depthBufferTexture", depthBufferTexture);
}

void BasicLightingRenderStage::initialize()
{
    setupGLState();
    loadShader();
    setupFbo();
    setupUniforms();

    static const auto zNear = 0.3f, zFar = 30.f;

    projection.data()->setZNear(zNear);
    projection.data()->setZFar(zFar);

    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({0.6f, 0.6f, 0.6f});
    m_grid->setNearFar(zNear, zFar);

    DebugMessage::enable();
}
void BasicLightingRenderStage::setupGLState()
{
    glClearColor(0.f, 0.f, 0.f, 1.0f);
}

void BasicLightingRenderStage::loadShader()
{
#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/massivelighting/basiclighting/icosahedron.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/massivelighting/basiclighting/icosahedron.frag")
    );
}

void BasicLightingRenderStage::resizeFbos(int width, int height)
{
    colorTexture.data()->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    idTexture.data()->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    normalTexture.data()->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    geometryTexture.data()->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    depthBufferTexture.data()->image2D(0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);

    m_fbo->printStatus(true);
}

void BasicLightingRenderStage::setupFbo()
{
    static const auto createTexture = [] (const std::string & name)
    {
        auto tex = Texture::createDefault(GL_TEXTURE_2D);
        tex->setName(name);
        return tex;
    };

    colorTexture.data() = createTexture("Color Texture");
    idTexture.data() = createTexture("ID Texture");
    normalTexture.data() = createTexture("Normal Texture");
    geometryTexture.data() = createTexture("Geometry Texture");
    depthBufferTexture.data() = createTexture("Depth Texture");
    m_fbo = make_ref<globjects::Framebuffer>();
    m_fbo->setName("Render FBO");

    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, colorTexture.data());
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT1, idTexture.data());
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT2, normalTexture.data());
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT3, geometryTexture.data());
    m_fbo->attachTexture(GL_DEPTH_STENCIL_ATTACHMENT, depthBufferTexture.data());
}

void BasicLightingRenderStage::setupUniforms()
{
    m_uniforms.addUniform(new globjects::Uniform<glm::mat4>("transform", glm::mat4()));
	m_uniforms.addUniform(new globjects::Uniform<glm::vec3>("eye", glm::vec3()));
	m_uniforms.addUniform(new globjects::Uniform<GLint>("material", 0));
	m_uniforms.addToProgram(m_program);
}

void BasicLightingRenderStage::process()
{
    auto rerender = false;

    if (drawables.hasChanged())
    {
        rerender = true;
    }
    if (lightsBuffer.hasChanged())
    {
        auto uniformBlock = m_program->uniformBlock("Lights");
        lightsBuffer.data()->bindBase(GL_UNIFORM_BUFFER, 0);
        uniformBlock->setBinding(0);
        rerender = true;
    }

    if (viewport.hasChanged())
    {
        resizeFbos(viewport.data()->width(), viewport.data()->height());
        rerender = true;
    }

    if (camera.hasChanged() || projection.hasChanged())
    {
        const auto transform = projection.data()->projection() * camera.data()->view();
        const auto eye = camera.data()->eye();
        m_uniforms.uniform<glm::mat4>("transform")->set(transform);
		m_uniforms.uniform<glm::vec3>("eye")->set(eye);

        m_grid->update(eye, transform);
        rerender = true;
    }

    if (rerender)
    {
        render();

        invalidateOutputs();
    }
}


void BasicLightingRenderStage::render()
{
    glViewport(
        viewport.data()->x(),
        viewport.data()->y(),
        viewport.data()->width(),
        viewport.data()->height());


    m_fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    m_grid->draw();

    m_program->use();

	for (auto i = 0; i < drawables->size(); ++i)
    {
		auto & drawable = drawables->at(i);
		auto & material = materials->at(i);

		if (material)
			material->bindActive(GL_TEXTURE0);
        
		drawable->draw();

		if (material)
			material->unbindActive(GL_TEXTURE0);
    }

    m_program->release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}




