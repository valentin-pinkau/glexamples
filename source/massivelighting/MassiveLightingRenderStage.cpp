#include "MassiveLightingRenderStage.h"

#include <glbinding/gl/bitfield.h>
#include <glbinding/callbacks.h>

#include <globjects/base/File.h>
#include <globjects/DebugMessage.h>

#include <globjects/Shader.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Framebuffer.h>
#include <globjects/globjects.h>

#include <reflectionzeug/Property.h>

#include <gloperate/primitives/UniformGroup.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractVirtualTimeCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/Camera.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/primitives/Icosahedron.h>
#include <gloperate/primitives/VertexDrawable.h>



using namespace gl;
using namespace globjects;



MassiveLightingRenderStage::MassiveLightingRenderStage()
:   AbstractStage("MassiveLightingRenderStage")
{
    addInput("viewport", viewport);
    addInput("camera", camera);
    addInput("projection", projection);

    addOutput("colorTexture", colorTexture);
    addOutput("idTexture", idTexture);
    addOutput("normalTexture", normalTexture);
    addOutput("geometryTexture", geometryTexture);
    addOutput("depthBufferTexture", depthBufferTexture);
}

void MassiveLightingRenderStage::initialize()
{
    setupGLState();
    loadShader();
    setupFbo();
    setupUniforms();
    m_icosahedron = new gloperate::Icosahedron{3};

    static const auto zNear = 0.3f, zFar = 15.f;

    projection.data()->setZNear(zNear);
    projection.data()->setZFar(zFar);

    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({0.6f, 0.6f, 0.6f});
    m_grid->setNearFar(zNear, zFar);


    DebugMessage::enable();

}
void MassiveLightingRenderStage::setupGLState()
{
    globjects::init();
    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

}

void MassiveLightingRenderStage::loadShader()
{
#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/massivelighting/icosahedron.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/massivelighting/icosahedron.frag")
    );
}

void MassiveLightingRenderStage::resizeFbos(int width, int height)
{
    colorTexture.data()->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    idTexture.data()->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    normalTexture.data()->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    geometryTexture.data()->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    depthBufferTexture.data()->image2D(0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);

    m_fbo->printStatus(true);
}

void MassiveLightingRenderStage::setupFbo()
{
    const auto createTexture = [] (const std::string & name)
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

void MassiveLightingRenderStage::setupUniforms()
{
    m_uniforms.addUniform(new globjects::Uniform<glm::mat4>("transform", glm::mat4()));
    m_uniforms.addToProgram(m_program);

}

void MassiveLightingRenderStage::process()
{
    auto rerender = false;

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

        m_grid->update(eye, transform);
        rerender = true;
    }

    if (rerender)
    {
        render();

        invalidateOutputs();
    }
}




void MassiveLightingRenderStage::render()
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

    m_icosahedron->draw();

    m_program->release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}



