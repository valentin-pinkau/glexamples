#pragma once

#include "MassiveLightingPipeline.h"

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/PipelinePainter.h>

namespace globjects
{
    class Program;
}

namespace gloperate
{
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}

class MassiveLightingPainter : public gloperate::PipelinePainter
{
public:
	MassiveLightingPainter(gloperate::ResourceManager & resourceManager, const std::string & relDataPath);
    virtual ~MassiveLightingPainter() = default;

protected:
    MassiveLightingPipeline m_pipeline;

    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

};
