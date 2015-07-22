#pragma once

#include <memory>
#include <vector>
#include <map>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>
#include <gloperate/primitives/PolygonalDrawable.h>
#include <gloperate/primitives/Light.h>

#include <reflectionzeug/base/FilePath.h>

#include "GPULights.h"

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

    gloperate::InputSlot<reflectionzeug::FilePath> sceneFilePath;
	gloperate::InputSlot<gloperate::ResourceManager *> resourceManager;
	gloperate::InputSlot<bool> useLightInput;
	gloperate::InputSlot<GPULights> lightsInput;
    gloperate::Data<std::vector<std::unique_ptr<gloperate::PolygonalDrawable>>> drawables;
	gloperate::Data<std::vector<globjects::ref_ptr<globjects::Texture>>> materials;
    gloperate::Data<globjects::ref_ptr<globjects::Buffer>> lightsBuffer;
	gloperate::Data<GPULights> gpuLights;

protected:
    virtual void process() override;
	void reloadScene();
	GPULights m_sceneLights;
};


