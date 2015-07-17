#include "BasicLightingPainter.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>
#include <gloperate/pipeline/AbstractPipeline.h>

using gloperate::make_unique;

BasicLightingPainter::BasicLightingPainter(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
    : PipelinePainter("BasicLighting", resourceManager, relDataPath, m_pipeline)
    , m_targetFramebufferCapability{addCapability(new gloperate::TargetFramebufferCapability())}
    , m_viewportCapability{addCapability(new gloperate::ViewportCapability())}
    , m_projectionCapability{addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability))}
    , m_cameraCapability{addCapability(new gloperate::CameraCapability(glm::vec3(-1.5, 2, 0), glm::vec3(5, 3, 0)))}
{
    m_pipeline.viewport.setData(m_viewportCapability);
    m_pipeline.projection.setData(m_projectionCapability);
    m_pipeline.camera.setData(m_cameraCapability);
    m_pipeline.targetFBO.setData(m_targetFramebufferCapability);
	m_pipeline.resourceManager.setData(&resourceManager);

    m_viewportCapability->changed.connect([this]() { m_pipeline.viewport.invalidate(); });
    m_projectionCapability->changed.connect([this]() { m_pipeline.projection.invalidate(); });
    m_cameraCapability->changed.connect([this]() { m_pipeline.camera.invalidate(); });
    m_targetFramebufferCapability->changed.connect([this]() { m_pipeline.targetFBO.invalidate(); });

    reflectionzeug::PropertyGroup * sceneGroup = addGroup("Scene");

    sceneGroup->addProperty(createProperty("SceneFilePath", m_pipeline.sceneFilePath));
}





