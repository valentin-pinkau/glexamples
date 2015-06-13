#pragma once
#include <vector>

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
    class Texture;
}

class MassiveLightingClusterStage : public gloperate::AbstractStage
{
    static const int xResolution = 4, yResolution = 4,  zResolution = 4;
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
    void createCluster();

protected:
    globjects::ref_ptr<globjects::Texture> m_clusterTexture;
    std::vector<int> m_cluster[xResolution][yResolution][zResolution];
    std::vector<int> m_indices;
    glm::ivec2 m_lookUp[xResolution * yResolution * zResolution];

};
