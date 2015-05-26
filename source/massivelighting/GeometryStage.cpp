#include "GeometryStage.h"

#include <glbinding/gl/gl.h>

#include <globjects/Texture.h>
#include <globjects/DebugMessage.h>
#include <globjects/logging.h>

#include <gloperate/base/make_unique.hpp>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate/primitives/Scene.h>
#include <gloperate/primitives/PolygonalDrawable.h> 
#include <gloperate/primitives/PolygonalGeometry.h>
#include <gloperate/primitives/Light.h>

#include <iostream>

using namespace gl;
using gloperate::make_unique;

GeometryStage::GeometryStage()
:   AbstractStage("GeometryStage")
{
    addInput("sceneFilePath", sceneFilePath);

    addOutput("drawables", drawables);
	addOutput("materials", materials);
    addOutput("lights", lights);
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
	const auto &sceneFile = sceneFilePath.data().string();
    const auto scene = resourceManager.data()->load<gloperate::Scene>(sceneFile);

    if (!scene)
    {
        globjects::debug() << "Could not load file";
        return;
    }

	// try to extract directory from scene file path
	std::string sceneDirectory;
	const auto lastSlashIndex = sceneFile.rfind('/');
	if (lastSlashIndex != std::string::npos)
		sceneDirectory = sceneFile.substr(0, lastSlashIndex + 1);

	// upload all textures
	std::map<unsigned int, globjects::ref_ptr<globjects::Texture>> materialsMap;
	for (const auto & material : scene->materials())
	{
		auto texturePath = sceneDirectory + material.second;
		auto texture = resourceManager.data()->load<globjects::Texture>(texturePath);

		if (texture == nullptr)
		{
			std::cout << "Could not load texture file " << texturePath << std::endl;
			continue;
		}

		texture->bind();
		texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
		texture->unbind();

		materialsMap[material.first] = texture;
	}

	drawables->clear();
	materials->clear();
	lights->clear();

	for (const auto & geometry : scene->meshes())
	{
		drawables->push_back(make_unique<gloperate::PolygonalDrawable>(*geometry));
		materials->push_back(materialsMap[geometry->materialIndex()]);
	}
    for (const auto light : scene->lights())
    {
        lights->push_back(make_unique<gloperate::Light>(*light));
    }

	delete scene;
}

