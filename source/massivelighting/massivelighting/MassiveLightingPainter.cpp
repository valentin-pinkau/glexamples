#include "MassiveLightingPainter.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>
#include <gloperate/pipeline/AbstractPipeline.h>

using gloperate::make_unique;

MassiveLightingPainter::MassiveLightingPainter(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
    : PipelinePainter("MassiveLighting", resourceManager, relDataPath, m_pipeline)
{
	auto targetFramebufferCapability = addCapability(new gloperate::TargetFramebufferCapability());
	auto viewportCapability = addCapability(new gloperate::ViewportCapability());
	auto projectionCapability = addCapability(new gloperate::PerspectiveProjectionCapability(viewportCapability));
	auto cameraCapability = addCapability(new gloperate::CameraCapability(glm::vec3(0, 2, -1.5), glm::vec3(0, 0, 0)));

    m_pipeline.viewport.setData(viewportCapability);
    m_pipeline.projection.setData(projectionCapability);
    m_pipeline.camera.setData(cameraCapability);
    m_pipeline.targetFBO.setData(targetFramebufferCapability);
	m_pipeline.resourceManager.setData(&resourceManager);

    viewportCapability->changed.connect([this]() { m_pipeline.viewport.invalidate(); });
    projectionCapability->changed.connect([this]() { m_pipeline.projection.invalidate(); });
    cameraCapability->changed.connect([this]() { m_pipeline.camera.invalidate(); });
    targetFramebufferCapability->changed.connect([this]() { m_pipeline.targetFBO.invalidate(); });

    reflectionzeug::PropertyGroup * sceneGroup = addGroup("Scene");

    sceneGroup->addProperty(createProperty("SceneFilePath", m_pipeline.sceneFilePath));
}