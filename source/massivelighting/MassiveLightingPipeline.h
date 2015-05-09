#pragma once

#include <gloperate/pipeline/AbstractPipeline.h>
#include <gloperate/pipeline/Data.h>

#include <reflectionzeug/FilePath.h>

namespace gloperate
{

class AbstractTargetFramebufferCapability;
class AbstractPerspectiveProjectionCapability;
class AbstractCameraCapability;
class AbstractViewportCapability;
class AbstractVirtualTimeCapability;
class AbstractTypedRenderTargetCapability;
class MetaInformationCapability;
class ResourceManager;

}

class MassiveLightingPipeline : public gloperate::AbstractPipeline
{
public:
	MassiveLightingPipeline(gloperate::ResourceManager & resourceManager);
    virtual ~MassiveLightingPipeline() = default;

public:
    gloperate::Data<gloperate::AbstractViewportCapability *> viewport;
    gloperate::Data<gloperate::AbstractPerspectiveProjectionCapability *> projection;
    gloperate::Data<gloperate::AbstractCameraCapability *> camera;
    gloperate::Data<gloperate::AbstractTargetFramebufferCapability *> targetFBO;

    gloperate::Data<reflectionzeug::FilePath> sceneFilePath;
};
