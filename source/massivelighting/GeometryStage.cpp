#include "GeometryStage.h"

#include "assimp/AssimpLoader.h"
#include "assimp/AssimpProcessing.h"

#include <assimp/cimport.h>

#include <globjects/DebugMessage.h>
#include <globjects/logging.h>

#include <gloperate/base/make_unique.hpp>

using gloperate::make_unique;

GeometryStage::GeometryStage()
:   AbstractStage("GeometryStage")
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
    auto assimpLoader = AssimpLoader{};
    const auto scene = assimpLoader.load(sceneFilePath.data().string(), {});

    if (!scene)
    {
        globjects::debug() << "Could not load file";
        return;
    }
    const auto geometries = AssimpProcessing::convertToGeometries(scene);

    aiReleaseImport(scene);
    *drawables = std::vector<std::unique_ptr<PolygonalDrawable>>();
    for (const auto & geometry : geometries)
        drawables->push_back(make_unique<PolygonalDrawable>(geometry));
}

