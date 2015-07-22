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

struct Plane
{
	glm::vec3 pointOnPlane;
	glm::vec3 normal;
};

class MassiveLightingClusterStage : public gloperate::AbstractStage
{
    //static const int xResolution = 64, yResolution = 32,  zResolution = 16;
public:
    MassiveLightingClusterStage();
    virtual ~MassiveLightingClusterStage() = default;
    virtual void initialize() override;

	gloperate::InputSlot<GPULights> gpuLights;
	gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
	gloperate::InputSlot<gloperate::AbstractProjectionCapability *> projection;
	gloperate::InputSlot<unsigned> xResolution;
	gloperate::InputSlot<unsigned> yResolution;
	gloperate::InputSlot<unsigned> zResolution;
	gloperate::InputSlot<float> attenuationThreshold;
	
	gloperate::Data<globjects::ref_ptr<globjects::Texture>> clusterTexture;
	gloperate::Data<globjects::ref_ptr<globjects::Texture>> lightIndicesTexture;

protected:
	void updateLightRadiuses(const GPULights & lights);
	void updatePlanes();
	virtual void process() override;
	
	void createClusters(const GPULights & lights);
	glm::vec3 projectToZPlane(const glm::vec3 & point, const Plane & plane);
	glm::vec3 projectToYPlane(const glm::vec3 & point, const Plane & plane);
	float getDistance(const glm::vec3 & point, const Plane & plane);
	std::array<float, MAX_LIGHTS> m_lightRadiuses;

	std::vector<Plane> x_planes;
	std::vector<Plane> y_planes;
	std::vector<Plane> z_planes;

	std::vector<unsigned> m_lightCounts;
    std::vector<std::array<glm::int16, 128>> m_cluster;
	std::vector<glm::uvec2> m_lookUp;
	std::vector<glm::uint16> m_indices;

	static const size_t m_lookupTextureWidth = 4096;
};
