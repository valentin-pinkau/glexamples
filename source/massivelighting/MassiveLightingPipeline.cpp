#include "MassiveLightingPipeline.h"
#include "MassiveLightingRenderStage.h"
#include "BlitStage.h"

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
    auto blitStage = make_unique<BlitStage>();

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;

    blitStage->viewport = viewport;
    blitStage->texture = renderStage->colorTexture;
    blitStage->targetFBO = targetFBO;


    addStages(
        std::move(renderStage),
        std::move(blitStage)
    );
}

MassiveLightingPipeline::~MassiveLightingPipeline()
{
}
