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

{
    //addParameter("strategies", &strategies);


    auto renderStage = make_unique<MassiveLightingRenderStage>();
    auto postprocessingStage = make_unique<PostprocessingStage>();

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;

    postprocessingStage->viewport = viewport;
    postprocessingStage->colorTexture = renderStage->colorTexture;
    postprocessingStage->depthTexture = renderStage->depthBufferTexture;
    postprocessingStage->targetFBO = targetFBO;


    addStages(
        std::move(renderStage),
        std::move(postprocessingStage)
    );
}

