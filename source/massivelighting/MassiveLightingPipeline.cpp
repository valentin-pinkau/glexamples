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

MassiveLightingPipeline::MassiveLightingPipeline()
: AbstractPipeline("MassiveLightingPipeline")
, sceneFilePath("data/transparency/transparency_scene.obj")

{
    //addParameter("strategies", &strategies);

    auto geometryStage = make_unique<GeometryStage>();
    auto renderStage = make_unique<MassiveLightingRenderStage>();
    auto postprocessingStage = make_unique<PostprocessingStage>();

    geometryStage->sceneFilePath = sceneFilePath;

    renderStage->drawables = geometryStage->drawables;
    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;

    postprocessingStage->viewport = viewport;
    postprocessingStage->colorTexture = renderStage->colorTexture;
    postprocessingStage->depthTexture = renderStage->depthBufferTexture;
    postprocessingStage->targetFBO = targetFBO;


    addStages(
        std::move(geometryStage),
        std::move(renderStage),
        std::move(postprocessingStage)
    );
}

