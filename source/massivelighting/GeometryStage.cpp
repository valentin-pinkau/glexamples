#include "GeometryStage.h"

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QImage>

#include <glbinding/gl/gl.h>

#include <globjects/Texture.h>
#include <globjects/DebugMessage.h>
#include <globjects/logging.h>

#include <gloperate/base/make_unique.hpp>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate/primitives/Scene.h>
#include <gloperate/primitives/PolygonalDrawable.h> 
#include <gloperate/primitives/PolygonalGeometry.h>

#include "qt/qgl.h"

using namespace gl;
using gloperate::make_unique;

GeometryStage::GeometryStage(gloperate::ResourceManager & resourceManager)
:   AbstractStage("GeometryStage")
,	m_resourceManager(resourceManager)
{
    addInput("sceneFilePath", sceneFilePath);

    addOutput("drawables", drawables);
	addOutput("materials", materials);
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

	// upload all textures
	std::map<unsigned int, globjects::ref_ptr<globjects::Texture>> materialsMap;
	auto sceneFile = QString::fromStdString(sceneFilePath.data().string());
	auto sceneDirectory = QFileInfo(sceneFile).dir();
	for (const auto & material : scene->materials())
	{
		auto materialFile = QString::fromStdString(material.second);
		QImage originalImage(sceneDirectory.filePath(materialFile));
		auto glImage = QGLWidget::convertToGLFormat(originalImage);

		globjects::ref_ptr<globjects::Texture> texture = globjects::Texture::createDefault(GL_TEXTURE_2D);
		texture->bind();
		texture->image2D(0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
		texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
		texture->unbind();

		materialsMap[material.first] = texture;
	}

	drawables->clear();
	materials->clear();

	for (const auto & geometry : scene->meshes())
	{
		drawables->push_back(make_unique<gloperate::PolygonalDrawable>(*geometry));
		materials->push_back(materialsMap[geometry->materialIndex()]);
	}

	delete scene;
}

