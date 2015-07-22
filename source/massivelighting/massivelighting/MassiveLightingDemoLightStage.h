#pragma once
#include <vector>
#include <array>

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>

#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/AbstractVirtualTimeCapability.h>

#include "GPULights.h"

struct PointLightInitTransform
{
	// Cylindrical coordinates
	float radius;
	float angle;
	float height;
	float animationSpeed;
};

class MassiveLightingDemoLightStage : public gloperate::AbstractStage
{
public:
	MassiveLightingDemoLightStage();
	virtual ~MassiveLightingDemoLightStage() = default;
    virtual void initialize() override;
	virtual void process() override;

	gloperate::InputSlot<gloperate::AbstractVirtualTimeCapability *> time;
	gloperate::InputSlot<unsigned> activeLights;
	gloperate::Data<GPULights> gpuLights;	

protected:
	glm::vec3 hueToRgb(float hue);
	std::vector<PointLightInitTransform> m_lightInitialTransform;
};
