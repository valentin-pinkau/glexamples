#pragma once

#include "BasicLightingPipeline.h"

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

class BasicLightingPainter : public gloperate::PipelinePainter
{
public:
	BasicLightingPainter(gloperate::ResourceManager & resourceManager, const std::string & relDataPath);
    virtual ~BasicLightingPainter() = default;

protected:
    BasicLightingPipeline m_pipeline;

    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

};
