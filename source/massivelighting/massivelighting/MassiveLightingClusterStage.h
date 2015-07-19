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
	void updatePlanes();
	virtual void process() override;
	
	void createClusters();
	glm::vec3 projectToZPlane(const glm::vec3 & point, const Plane & plane);
	glm::vec3 projectToYPlane(const glm::vec3 & point, const Plane & plane);
	float getDistance(const glm::vec3 & point, const Plane & plane);
	std::vector<float> m_lightRadiuses;

	std::array<Plane, xResolution + 1> x_planes;
	std::array<Plane, yResolution + 1> y_planes;
	std::array<Plane, zResolution + 1> z_planes;

	int m_lightCounts[xResolution][yResolution][zResolution];
    std::array<glm::int16, 32> m_cluster[xResolution][yResolution][zResolution];
    glm::uvec2 m_lookUp[xResolution * yResolution * zResolution];
	std::vector<glm::uint16> m_indices;
};
