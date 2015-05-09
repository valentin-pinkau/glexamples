#include "GeometryStage.h"

#include <assimp/cimport.h>

#include <globjects/DebugMessage.h>
#include <globjects/logging.h>

#include <gloperate/base/make_unique.hpp>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate/primitives/Scene.h>
#include <gloperate/primitives/PolygonalDrawable.h> 

using gloperate::make_unique;

GeometryStage::GeometryStage(gloperate::ResourceManager & resourceManager)
:   AbstractStage("GeometryStage")
,	m_resourceManager(resourceManager)
{
    addInput("sceneFilePath", sceneFilePath);

    addOutput("drawables", drawables);
}

void GeometryStage::initialize()
{
    globjects::DebugMessage::enable();
}
void GeometryStage::process()
{
    auto reload = false;

    if(sceneFilePath.hasChanged() && sceneFilePath.data().string() != "")
    {
        reload = true;
    }

    if(reload)
    {
        reloadScene();

        invalidateOutputs();
    }

}

void GeometryStage::reloadScene()
{
    const auto scene = m_resourceManager.load<gloperate::Scene>(sceneFilePath.data().string());

    if (!scene)
    {
        globjects::debug() << "Could not load file";
        return;
    }

    *drawables = std::vector<std::unique_ptr<gloperate::PolygonalDrawable>>();
    for (const auto & geometry : scene->meshes())
        drawables->push_back(make_unique<gloperate::PolygonalDrawable>(*geometry));

	delete scene;
}

