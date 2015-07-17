#include "GeometryStage.h"
#include "BasicLightingPipeline.h"
#include "BasicLightingRenderStage.h"
#include "BasicLightingPostprocessingStage.h"

#include <gloperate/base/make_unique.hpp>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/resources/ResourceManager.h>
#include <globjects/base/File.h>
#include <globjects/NamedString.h>

using gloperate::make_unique;

BasicLightingPipeline::BasicLightingPipeline()
: AbstractPipeline("BasicLightingPipeline")
, sceneFilePath("data/massivelighting/models/testScene/testScene.fbx")

{
    auto geometryStage = new GeometryStage();
    auto renderStage = new BasicLightingRenderStage();
    auto postprocessingStage = new BasicLightingPostprocessingStage();

    geometryStage->sceneFilePath = sceneFilePath;
	geometryStage->resourceManager = resourceManager;

    renderStage->drawables = geometryStage->drawables;
	renderStage->materials = geometryStage->materials;
    renderStage->lightsBuffer = geometryStage->lightsBuffer;
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

	// Initialize shader includes
	globjects::NamedString::create("data/massivelighting/shaders/common/phong.glsl", new globjects::File("data/massivelighting/shaders/common/phong.glsl"));
}

