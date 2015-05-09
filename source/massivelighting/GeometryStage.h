#pragma once

#include <memory>
#include <vector>

#include <glm/vec3.hpp>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/primitives/PolygonalDrawable.h>

#include <reflectionzeug/FilePath.h>

namespace gloperate
{
	class ResourceManager;
}


class GeometryStage : public gloperate::AbstractStage
{
public:
	GeometryStage(gloperate::ResourceManager & resourceManager);
    virtual ~GeometryStage() = default;
    virtual void initialize() override;

public:
    gloperate::InputSlot<reflectionzeug::FilePath> sceneFilePath;
    gloperate::Data<std::vector<std::unique_ptr<gloperate::PolygonalDrawable>>> drawables;

protected:
    virtual void process() override;
	void reloadScene();
	gloperate::ResourceManager & m_resourceManager;

};


