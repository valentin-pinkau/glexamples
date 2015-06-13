#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>

#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>

#include "GPULights.h"

namespace globjects
{
	class Buffer;
}

class MassiveLightingClusterStage : public gloperate::AbstractStage
{
public:
    MassiveLightingClusterStage();
    virtual ~MassiveLightingClusterStage() = default;
    virtual void initialize() override;

public:
	gloperate::InputSlot<GPULights> gpuLights;
	gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
	gloperate::InputSlot<gloperate::AbstractProjectionCapability *> projection;

protected:
    virtual void process() override;

protected:

};