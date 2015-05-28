#include "GeometryStage.h"
#include "MassiveLightingPipeline.h"
#include "MassiveLightingRenderStage.h"
#include "MassiveLightingPostprocessingStage.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/resources/ResourceManager.h>

using gloperate::make_unique;

MassiveLightingPipeline::MassiveLightingPipeline()
: AbstractPipeline("MassiveLightingPipeline")
, sceneFilePath("data/massivelighting/testScene/testScene.fbx")

{
    //addParameter("strategies", &strategies);

    auto geometryStage = new GeometryStage();
    auto renderStage = new MassiveLightingRenderStage();
    auto postprocessingStage = new MassiveLightingPostprocessingStage();

    geometryStage->sceneFilePath = sceneFilePath;
	geometryStage->resourceManager = resourceManager;

    renderStage->drawables = geometryStage->drawables;
	renderStage->materials = geometryStage->materials;
    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;

    postprocessingStage->viewport = viewport;
	postprocessingStage->camera = camera;
	postprocessingStage->projection = projection;
	postprocessingStage->colorTexture = renderStage->colorTexture;
	postprocessingStage->normalTexture = renderStage->normalTexture;
    postprocessingStage->depthTexture = renderStage->depthBufferTexture;
    postprocessingStage->targetFBO = targetFBO;


    addStages(
        geometryStage,
        renderStage,
        postprocessingStage
    );
}

