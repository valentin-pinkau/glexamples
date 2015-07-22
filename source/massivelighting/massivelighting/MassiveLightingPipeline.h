#pragma once

#include <gloperate/pipeline/AbstractPipeline.h>
#include <gloperate/pipeline/Data.h>

#include <reflectionzeug/base/FilePath.h>

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
	MassiveLightingPipeline();
    virtual ~MassiveLightingPipeline() = default;

public:
    gloperate::Data<gloperate::AbstractViewportCapability *> viewport;
    gloperate::Data<gloperate::AbstractPerspectiveProjectionCapability *> projection;
    gloperate::Data<gloperate::AbstractCameraCapability *> camera;
    gloperate::Data<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
	gloperate::Data<gloperate::AbstractVirtualTimeCapability *> time;
	gloperate::Data<gloperate::ResourceManager *> resourceManager;
	

    gloperate::Data<reflectionzeug::FilePath> sceneFilePath;
	gloperate::Data<bool> enableDebugView;
	gloperate::Data<bool> enableDemoLights;
	gloperate::Data<unsigned> demoLightsCount;
	gloperate::Data<bool> animateLights;
	gloperate::Data<unsigned> clustersX;
	gloperate::Data<unsigned> clustersY;
	gloperate::Data<unsigned> clustersZ;
	gloperate::Data<float> attenuationThreshold;
};
