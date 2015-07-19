#pragma once
#include <vector>
#include <array>

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
    static const int xResolution = 64, yResolution = 64,  zResolution = 16;
public:
    MassiveLightingClusterStage();
    virtual ~MassiveLightingClusterStage() = default;
    virtual void initialize() override;

	gloperate::InputSlot<GPULights> gpuLights;
	gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
	gloperate::InputSlot<gloperate::AbstractProjectionCapability *> projection;
	gloperate::Data<globjects::ref_ptr<globjects::Texture>> clusterTexture;
	gloperate::Data<globjects::ref_ptr<globjects::Texture>> lightIndicesTexture;

protected:
	void updateLightRadiuses();
	virtual void process() override;
	float z_planes(unsigned i);
	float y_planes(unsigned i);
	float x_planes(unsigned i);
	void createCluster();

	std::vector<float> m_lightRadiuses;

	int m_lightCounts[xResolution][yResolution][zResolution];
    std::array<glm::int16, 128> m_cluster[xResolution][yResolution][zResolution];
    glm::ivec2 m_lookUp[xResolution * yResolution * zResolution];
	std::vector<glm::uint16> m_indices;
};
