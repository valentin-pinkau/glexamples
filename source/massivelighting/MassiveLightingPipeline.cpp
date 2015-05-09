#include "GeometryStage.h"
#include "MassiveLightingPipeline.h"
#include "MassiveLightingRenderStage.h"
#include "PostprocessingStage.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>

using gloperate::make_unique;

MassiveLightingPipeline::MassiveLightingPipeline(gloperate::ResourceManager & resourceManager)
: AbstractPipeline("MassiveLightingPipeline")
, sceneFilePath("data/transparency/transparency_scene.obj")

{
    //addParameter("strategies", &strategies);

    auto geometryStage = new GeometryStage(resourceManager);
    auto renderStage = new MassiveLightingRenderStage();
    auto postprocessingStage = new PostprocessingStage();

    geometryStage->sceneFilePath = sceneFilePath;

    renderStage->drawables = geometryStage->drawables;
	renderStage->materials = geometryStage->materials;
    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;

    postprocessingStage->viewport = viewport;
    postprocessingStage->colorTexture = renderStage->colorTexture;
    postprocessingStage->depthTexture = renderStage->depthBufferTexture;
    postprocessingStage->targetFBO = targetFBO;


    addStages(
        geometryStage,
        renderStage,
        postprocessingStage
    );
}

