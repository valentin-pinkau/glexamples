#include "StochasticTransparency.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <glbinding/gl/boolean.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/Framebuffer.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>

#include <gloperate/base/RenderTargetType.h>
#include <gloperate/base/make_unique.hpp>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/primitives/Scene.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/primitives/PolygonalGeometry.h>

#include <reflectionzeug/property/PropertyGroup.h>

#include <widgetzeug/make_unique.hpp>

#include "MasksTableGenerator.h"
#include "StochasticTransparencyOptions.h"


using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

StochasticTransparency::StochasticTransparency(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
:   Painter("StochasticTransparency", resourceManager, relDataPath)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
,   m_options(new StochasticTransparencyOptions(*this))
{
}

StochasticTransparency::~StochasticTransparency() = default;

void StochasticTransparency::onInitialize()
{
    globjects::init();
    globjects::DebugMessage::enable();

#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    m_options->initGL();
    
    m_grid = make_ref<gloperate::AdaptiveGrid>();
    m_grid->setColor({0.6f, 0.6f, 0.6f});
    
    setupPrograms();
    setupProjection();
    setupFramebuffer();
    setupMasksTexture();
    setupDrawable();
}

void StochasticTransparency::onPaint()
{
    if (m_viewportCapability->hasChanged())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width(),
            m_viewportCapability->height());

        m_viewportCapability->setChanged(false);
        
        const auto viewport = glm::vec2{m_viewportCapability->width(), m_viewportCapability->height()};
        m_alphaToCoverageProgram->setUniform("viewport", viewport);
        
        updateFramebuffer();
    }
    
    if (m_options->numSamplesChanged())
        updateNumSamples();
    
    clearBuffers();
    updateUniforms();
    
    if (m_options->optimization() == StochasticTransparencyOptimization::NoOptimization)
    {
        renderOpaqueGeometry();
        
        if (m_options->backFaceCulling())
            glEnable(GL_CULL_FACE);
        
        glEnable(GL_SAMPLE_SHADING);
        glMinSampleShading(1.0);
        
        renderAlphaToCoverage(kOpaqueColorAttachment);
        
        glDisable(GL_SAMPLE_SHADING);
        glDisable(GL_CULL_FACE);
        
        blit();
    }
    else
    {
        renderOpaqueGeometry();
        renderTransparentGeometry();
        composite();
    }
    
    Framebuffer::unbind(GL_FRAMEBUFFER);
}

void StochasticTransparency::setupFramebuffer()
{
    m_opaqueColorAttachment = make_ref<Texture>(GL_TEXTURE_2D_MULTISAMPLE);
    m_transparentColorAttachment = make_ref<Texture>(GL_TEXTURE_2D_MULTISAMPLE);
    m_totalAlphaAttachment = make_ref<Texture>(GL_TEXTURE_2D_MULTISAMPLE);
    m_depthAttachment = make_ref<Texture>(GL_TEXTURE_2D_MULTISAMPLE);
    
    updateFramebuffer();
    
    m_fbo = make_ref<Framebuffer>();
    
    m_fbo->attachTexture(kOpaqueColorAttachment, m_opaqueColorAttachment);
    m_fbo->attachTexture(kTransparentColorAttachment, m_transparentColorAttachment);
    m_fbo->attachTexture(kTotalAlphaAttachment, m_totalAlphaAttachment);
    m_fbo->attachTexture(GL_DEPTH_ATTACHMENT, m_depthAttachment);

    m_fbo->printStatus(true);
}

void StochasticTransparency::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 30.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void StochasticTransparency::setupDrawable()
{
    // Load scene
    const auto scene = m_resourceManager.load<gloperate::Scene>("data/transparency/transparency_scene.obj");
    if (!scene)
    {
        std::cout << "Could not load file" << std::endl;
        return;
    }

    // Create a renderable for each mesh
    for (const auto * geometry : scene->meshes()) {
        m_drawables.push_back(gloperate::make_unique<gloperate::PolygonalDrawable>(*geometry));
    }

    // Release scene
    delete scene;
}

void StochasticTransparency::setupPrograms()
{
    static const auto totalAlphaShaders = "total_alpha";
    static const auto alphaToCoverageShaders = "alpha_to_coverage";
    static const auto transparentColorsShaders = "transparent_colors";
    static const auto compositingShaders = "compositing";
    
    const auto initProgram = [] (globjects::ref_ptr<globjects::Program> & program, const char * shaders)
    {
        static const auto shaderPath = std::string{"data/transparency/"};
        
        program = make_ref<Program>();
        program->attach(
            Shader::fromFile(GL_VERTEX_SHADER, shaderPath + shaders + ".vert"),
            Shader::fromFile(GL_FRAGMENT_SHADER, shaderPath + shaders + ".frag"));
    };
    
    initProgram(m_totalAlphaProgram, totalAlphaShaders);
    initProgram(m_alphaToCoverageProgram, alphaToCoverageShaders);
    initProgram(m_colorAccumulationProgram, transparentColorsShaders);
    initProgram(m_compositingProgram, compositingShaders);
    
    m_alphaToCoverageProgram->setUniform("masksTexture", 0);
    
    updateNumSamplesUniforms();
    
    const auto opaqueColorLocation = m_compositingProgram->getUniformLocation("opaqueColorTexture");
    const auto totalAlphaLocation = m_compositingProgram->getUniformLocation("totalAlphaTexture");
    const auto transparentColorLocation = m_compositingProgram->getUniformLocation("transparentColorTexture");
    
    m_compositingProgram->setUniform(opaqueColorLocation, 0);
    m_compositingProgram->setUniform(totalAlphaLocation, 1);
    m_compositingProgram->setUniform(transparentColorLocation, 2);
    
    m_compositingQuad = make_ref<gloperate::ScreenAlignedQuad>(m_compositingProgram);
}

void StochasticTransparency::setupMasksTexture()
{
    static const auto numSamples = m_options->numSamples();
    const auto table = MasksTableGenerator::generateDistributions(numSamples);
    
    m_masksTexture = Texture::createDefault(GL_TEXTURE_2D);
    m_masksTexture->image2D(0, GL_R8, table->at(0).size(), table->size(), 0, GL_RED, GL_UNSIGNED_BYTE, table->data());
}

void StochasticTransparency::updateFramebuffer()
{
    const auto numSamples = m_options->numSamples();
    const auto size = glm::ivec2{m_viewportCapability->width(), m_viewportCapability->height()};
    
    m_opaqueColorAttachment->image2DMultisample(numSamples, GL_RGBA8, size, GL_FALSE);
    m_transparentColorAttachment->image2DMultisample(numSamples, GL_RGBA32F, size, GL_FALSE);
    m_totalAlphaAttachment->image2DMultisample(numSamples, GL_R32F, size, GL_FALSE);
    m_depthAttachment->image2DMultisample(numSamples, GL_DEPTH_COMPONENT, size, GL_FALSE);
}

void StochasticTransparency::updateNumSamples()
{
    setupMasksTexture();
    updateFramebuffer();
    updateNumSamplesUniforms();
}

void StochasticTransparency::updateNumSamplesUniforms()
{
    m_compositingProgram->setUniform("numSamples", static_cast<int>(m_options->numSamples()));
}

void StochasticTransparency::clearBuffers()
{
    m_fbo->setDrawBuffers({ kOpaqueColorAttachment, kTransparentColorAttachment, kTotalAlphaAttachment });
    
    m_fbo->clearBuffer(GL_COLOR, 0, glm::vec4(0.85f, 0.87f, 0.91f, 1.0f));
    m_fbo->clearBuffer(GL_COLOR, 1, glm::vec4(0.0f));
    m_fbo->clearBuffer(GL_COLOR, 2, glm::vec4(1.0f));
    m_fbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0.0f);
}

void StochasticTransparency::updateUniforms()
{
    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();
    const auto transparency = static_cast<unsigned int>(m_options->transparency());
    
    m_grid->update(eye, transform);
    
    auto updateProgramUniforms = [&transform, &transparency] (Program * program)
    {
        program->setUniform("transform", transform);
        program->setUniform("transparency", transparency);
    };
    
    updateProgramUniforms(m_totalAlphaProgram);
    updateProgramUniforms(m_alphaToCoverageProgram);
    updateProgramUniforms(m_colorAccumulationProgram);
}

void StochasticTransparency::renderOpaqueGeometry()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_fbo->bind(GL_FRAMEBUFFER);
    m_fbo->setDrawBuffer(kOpaqueColorAttachment);

    m_grid->draw();
}

void StochasticTransparency::renderTransparentGeometry()
{
    if (m_options->backFaceCulling())
        glEnable(GL_CULL_FACE);
    
    renderTotalAlpha();
    
    glEnable(GL_SAMPLE_SHADING);
    glMinSampleShading(1.0);

    if (m_options->optimization() == StochasticTransparencyOptimization::AlphaCorrection)
    {
        renderAlphaToCoverage(kTransparentColorAttachment);
    }
    else if (m_options->optimization() == StochasticTransparencyOptimization::AlphaCorrectionAndDepthBased)
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        renderAlphaToCoverage(kTransparentColorAttachment);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        
        renderColorAccumulation();
    }
    
    glDisable(GL_SAMPLE_SHADING);
    glDisable(GL_CULL_FACE);
}

void StochasticTransparency::renderTotalAlpha()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glEnable (GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    
    m_fbo->bind(GL_FRAMEBUFFER);
    m_fbo->setDrawBuffer(kTotalAlphaAttachment);
    
    m_totalAlphaProgram->use();
    
    for (auto & drawable : m_drawables)
        drawable->draw();
    
    m_totalAlphaProgram->release();
    
    glDisable(GL_BLEND);
}

void StochasticTransparency::renderAlphaToCoverage(gl::GLenum colorAttachment)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_fbo->bind(GL_FRAMEBUFFER);
    m_fbo->setDrawBuffer(colorAttachment);
    
    m_masksTexture->bindActive(GL_TEXTURE0);

    m_alphaToCoverageProgram->use();

    for (auto & drawable : m_drawables)
        drawable->draw();

    m_alphaToCoverageProgram->release();
}

void StochasticTransparency::renderColorAccumulation()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glDepthFunc(GL_LEQUAL);
    
    glEnable (GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    m_fbo->bind(GL_FRAMEBUFFER);
    m_fbo->setDrawBuffer(kTransparentColorAttachment);
    
    m_colorAccumulationProgram->use();
    
    for (auto & drawable : m_drawables)
        drawable->draw();
    
    m_colorAccumulationProgram->release();

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
}

void StochasticTransparency::blit()
{
    auto targetfbo = m_targetFramebufferCapability->framebuffer();
    auto drawBuffer = GL_COLOR_ATTACHMENT0;
    
    if (!targetfbo)
    {
        targetfbo = Framebuffer::defaultFBO();
        drawBuffer = GL_BACK_LEFT;
    }
    
    const auto rect = std::array<GLint, 4>{{
        m_viewportCapability->x(),
        m_viewportCapability->y(),
        m_viewportCapability->width(),
        m_viewportCapability->height()
    }};
    
    m_fbo->blit(kOpaqueColorAttachment, rect, targetfbo, drawBuffer, rect,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void StochasticTransparency::composite()
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    auto targetfbo = m_targetFramebufferCapability->framebuffer();
    
    if (!targetfbo)
        targetfbo = Framebuffer::defaultFBO();
    
    targetfbo->bind(GL_FRAMEBUFFER);
    
    m_opaqueColorAttachment->bindActive(GL_TEXTURE0);
    m_totalAlphaAttachment->bindActive(GL_TEXTURE1);
    m_transparentColorAttachment->bindActive(GL_TEXTURE2);
    
    m_compositingQuad->draw();
    
    const auto rect = std::array<GLint, 4>{{
        m_viewportCapability->x(),
        m_viewportCapability->y(),
        m_viewportCapability->width(),
        m_viewportCapability->height()
    }};

    m_fbo->blit(GL_COLOR_ATTACHMENT0, rect, targetfbo, GL_BACK_LEFT, rect, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
