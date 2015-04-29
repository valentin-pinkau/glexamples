#include "MassiveLightingPainter.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>
#include <gloperate/pipeline/AbstractPipeline.h>

using gloperate::make_unique;

MassiveLightingPainter::MassiveLightingPainter(gloperate::ResourceManager & resourceManager)
    : PipelinePainter(resourceManager, m_pipeline)
    , m_targetFramebufferCapability{addCapability(make_unique<gloperate::TargetFramebufferCapability>())}
    , m_viewportCapability{addCapability(make_unique<gloperate::ViewportCapability>())}
    , m_projectionCapability{addCapability(make_unique<gloperate::PerspectiveProjectionCapability>(m_viewportCapability))}
    , m_cameraCapability{addCapability(make_unique<gloperate::CameraCapability>())}
{
    m_pipeline.viewport.setData(m_viewportCapability);
    m_pipeline.projection.setData(m_projectionCapability);
    m_pipeline.camera.setData(m_cameraCapability);
    m_pipeline.targetFBO.setData(m_targetFramebufferCapability);

    m_viewportCapability->changed.connect([this]() { m_pipeline.viewport.invalidate(); });
    m_projectionCapability->changed.connect([this]() { m_pipeline.projection.invalidate(); });
    m_cameraCapability->changed.connect([this]() { m_pipeline.camera.invalidate(); });
    m_targetFramebufferCapability->changed.connect([this]() { m_pipeline.targetFBO.invalidate(); });
}





