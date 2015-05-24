#pragma once

#include <memory>
#include <vector>
#include <map>

#include <glm/vec3.hpp>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/primitives/PolygonalDrawable.h>

#include <reflectionzeug/FilePath.h>

namespace globjects
{
	class Texture;
}

namespace gloperate
{
	class ResourceManager;
}


class GeometryStage : public gloperate::AbstractStage
{
public:
	GeometryStage();
    virtual ~GeometryStage() = default;
    virtual void initialize() override;

public:
    gloperate::InputSlot<reflectionzeug::FilePath> sceneFilePath;
	gloperate::InputSlot<gloperate::ResourceManager *> resourceManager;
    gloperate::Data<std::vector<std::unique_ptr<gloperate::PolygonalDrawable>>> drawables;
	gloperate::Data<std::vector<globjects::ref_ptr<globjects::Texture>>> materials;

protected:
    virtual void process() override;
	void reloadScene();

};


