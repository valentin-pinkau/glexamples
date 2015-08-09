#include "GeometryStage.h"
#include "BasicLightingPipeline.h"
#include "BasicLightingRenderStage.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/resources/ResourceManager.h>
#include <globjects/base/File.h>


using gloperate::make_unique;

BasicLightingPipeline::BasicLightingPipeline()
: AbstractPipeline("BasicLightingPipeline")
, sceneFilePath("data/massivelighting/models/testScene/testScene.fbx")

{
    auto geometryStage = new GeometryStage();
    auto renderStage = new BasicLightingRenderStage();

    geometryStage->sceneFilePath = sceneFilePath;
	geometryStage->resourceManager = resourceManager;
    geometryStage->useLightInput = gloperate::Data<bool>(false);

    renderStage->drawables = geometryStage->drawables;
	renderStage->materials = geometryStage->materials;
    renderStage->lightsBuffer = geometryStage->lightsBuffer;
    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;
	renderStage->targetFBO = targetFBO;

    addStages(
        geometryStage,
        renderStage
    );


}

