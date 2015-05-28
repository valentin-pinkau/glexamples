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

class BasicLightingPipeline : public gloperate::AbstractPipeline
{
public:
	BasicLightingPipeline();
	virtual ~BasicLightingPipeline() = default;

public:
    gloperate::Data<gloperate::AbstractViewportCapability *> viewport;
    gloperate::Data<gloperate::AbstractPerspectiveProjectionCapability *> projection;
    gloperate::Data<gloperate::AbstractCameraCapability *> camera;
    gloperate::Data<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
	gloperate::Data<gloperate::ResourceManager *> resourceManager;

    gloperate::Data<reflectionzeug::FilePath> sceneFilePath;
};
