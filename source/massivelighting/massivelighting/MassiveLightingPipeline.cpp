#include "GeometryStage.h"
#include "MassiveLightingPipeline.h"
#include "MassiveLightingRenderStage.h"
#include "MassiveLightingClusterStage.h"
#include "MassiveLightingPostprocessingStage.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/resources/ResourceManager.h>
#include <globjects/base/File.h>
#include <globjects/NamedString.h>
#include "MassiveLightingDemoLightStage.h"

using gloperate::make_unique;

MassiveLightingPipeline::MassiveLightingPipeline()
	: AbstractPipeline("MassiveLightingPipeline")
	, sceneFilePath("data/massivelighting/models/crytek-sponza/sponza.fbx")
	, enableDebugView(false)
	, enableDemoLights(true)
	, demoLightsCount(64)
	, clustersX(64)
	, clustersY(32)
	, clustersZ(16)
	, attenuationThreshold(0.1f)
{
	auto demoLightStage = new MassiveLightingDemoLightStage();
    auto geometryStage = new GeometryStage();
    auto renderStage = new MassiveLightingRenderStage();
	auto clusterStage = new MassiveLightingClusterStage();
    auto postprocessingStage = new MassiveLightingPostprocessingStage();

	demoLightStage->activeLights = demoLightsCount;
	demoLightStage->time = time;

	geometryStage->useLightInput = enableDemoLights;
	geometryStage->lightsInput = demoLightStage->gpuLights;
    geometryStage->sceneFilePath = sceneFilePath;
	geometryStage->resourceManager = resourceManager;

    renderStage->drawables = geometryStage->drawables;
	renderStage->materials = geometryStage->materials;
    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;

	clusterStage->camera = camera;
	clusterStage->projection = projection;
	clusterStage->gpuLights = geometryStage->gpuLights;
	clusterStage->xResolution = clustersX;
	clusterStage->yResolution = clustersY;
	clusterStage->zResolution = clustersZ;
	clusterStage->attenuationThreshold = attenuationThreshold;

    postprocessingStage->viewport = viewport;
    postprocessingStage->lightsBuffer = geometryStage->lightsBuffer;
	postprocessingStage->camera = camera;
	postprocessingStage->projection = projection;
	postprocessingStage->colorTexture = renderStage->colorTexture;
	postprocessingStage->normalTexture = renderStage->normalTexture;
    postprocessingStage->depthTexture = renderStage->depthBufferTexture;
	postprocessingStage->clusterTexture = clusterStage->clusterTexture;
	postprocessingStage->lightIndicesTexture = clusterStage->lightIndicesTexture;
    postprocessingStage->targetFBO = targetFBO;
	postprocessingStage->enableDebugOutput = enableDebugView;

	addStages(
		demoLightStage,
        geometryStage,
        renderStage,
		clusterStage,
        postprocessingStage
    );

	// Initialize shader includes
	globjects::NamedString::create("data/massivelighting/shaders/common/phong.glsl", new globjects::File("data/massivelighting/shaders/common/phong.glsl"));
}

