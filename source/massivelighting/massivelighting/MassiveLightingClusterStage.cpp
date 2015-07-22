#include "MassiveLightingClusterStage.h"

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Logging.h>

using namespace gl;

MassiveLightingClusterStage::MassiveLightingClusterStage()
:   AbstractStage("MassiveLightingClusterStage")
{
	addInput("gpuLights", gpuLights);
	addInput("camera", camera);
	addInput("projection", projection);
	addInput("xResolution", xResolution);
	addInput("yResolution", yResolution);
	addInput("zResolution", zResolution);
	addInput("attenuationThreshold", attenuationThreshold);
}

void MassiveLightingClusterStage::initialize()
{
	auto lookupTexture = globjects::make_ref<globjects::Texture>(GL_TEXTURE_3D);
    lookupTexture->setName("cluster Texture");
    lookupTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    lookupTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    lookupTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    lookupTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    lookupTexture->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	clusterTexture.setData(lookupTexture);

    auto indicesTexture = globjects::make_ref<globjects::Texture>(GL_TEXTURE_2D);
    indicesTexture->setName("light indices Texture");
    indicesTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    indicesTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    indicesTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	
    lightIndicesTexture.setData(indicesTexture);
}

void MassiveLightingClusterStage::process()
{
    bool recluster = false;
	bool regeneratePlanes = false;

	if (xResolution.hasChanged() || yResolution.hasChanged() || zResolution.hasChanged())
	{
		auto x = xResolution.data(), y = yResolution.data(), z = zResolution.data();

		// Reserve space for 4 lights per cluster upfront
		m_indices.reserve(4 * x * y * z);

		x_planes.resize(x + 1);
		y_planes.resize(y + 1);
		z_planes.resize(z + 1);
		regeneratePlanes = true;

		m_lightCounts.resize(x*y*z);

		m_cluster.resize(x*y*z);
		m_lookUp.resize(x*y*z);
	}

    if (camera.hasChanged())
    {
        recluster = true;
    }

	if (gpuLights.hasChanged() || attenuationThreshold.hasChanged())
	{
		updateLightRadiuses(gpuLights.data());
		recluster = true;
	}

	if (projection.hasChanged())
	{
		regeneratePlanes = true;
		recluster = true;
	}

	if (regeneratePlanes)
	{
		updatePlanes();
	}

    if (recluster)
    {
        createClusters(gpuLights.data());

		clusterTexture.data()->image3D(0, GL_RG32UI, xResolution.data(), yResolution.data(), zResolution.data(),
			0, GL_RG_INTEGER, GL_UNSIGNED_INT, m_lookUp.data());
		
		auto height = m_indices.size() / m_lookupTextureWidth;
		if (m_indices.size() % m_lookupTextureWidth != 0) ++height;
		m_indices.reserve(m_lookupTextureWidth * height);

		lightIndicesTexture.data()->image2D(0, GL_R16UI, m_lookupTextureWidth, height, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, m_indices.data());
    }	
}

void MassiveLightingClusterStage::updateLightRadiuses(const GPULights & lights)
{
	for (auto i = 0; i < lights.number_of_lights; ++i)
	{
		auto & gpuLight = lights.lights[i];

		// Calculate distance where attenuation(d) = epsilon
		const float attenuation_epsilon = attenuationThreshold.data();
		// http://gamedev.stackexchange.com/questions/56897/glsl-light-attenuation-color-and-intensity-formula

		// Point light
		float linearAttenuation = gpuLight.attenuation.y;
		float quadraticAttenuation = gpuLight.attenuation.z;

		float distLinear = linearAttenuation > 0.f ? 1.f / (linearAttenuation * attenuation_epsilon) : std::numeric_limits<float>::infinity();
		float distQuadratic = quadraticAttenuation > 0.f ? sqrt(1.f / (quadraticAttenuation * attenuation_epsilon)) : std::numeric_limits<float>::infinity();
		m_lightRadiuses[i] = std::min(distLinear, distQuadratic);
	}
}

void MassiveLightingClusterStage::updatePlanes()
{
	auto xRes = xResolution.data(), yRes = yResolution.data(), zRes = zResolution.data();

	auto zNear = projection.data()->zNear();
	auto zFar = projection.data()->zFar();

	auto inverseProjection = projection.data()->projectionInverted();

	for (auto x = 0; x <= xRes; ++x)
	{
		auto sx = (float(x) / float(xRes)) * 2.f - 1.f;
		auto near = glm::vec4(sx, 0.f, -1.f, 1.f);
		auto far = glm::vec4(sx, 0.f, 1.f, 1.f);

		auto vNear = inverseProjection * near;
		vNear /= vNear.w;

		auto vFar = inverseProjection * far;
		vFar /= vFar.w;

		Plane p;
		p.pointOnPlane = glm::vec3(vNear) / -zNear;
		p.normal = glm::normalize(glm::cross(glm::vec3(vFar - vNear), glm::vec3(0, 1.f, 0)));

		x_planes[x] = p;
	}

	for (auto y = 0; y <= yRes; ++y)
	{
		auto sy = (float(y) / float(yRes)) * 2.f - 1.f;
		auto near = glm::vec4(0.f, sy, -1.f, 1.f);
		auto far = glm::vec4(0.f, sy, 1.f, 1.f);

		auto vNear = inverseProjection * near;
		vNear /= vNear.w;

		auto vFar = inverseProjection * far;
		vFar /= vFar.w;

		Plane p;
		p.pointOnPlane = glm::vec3(vNear) / -zNear;
		p.normal = glm::normalize(glm::cross(glm::vec3(vFar - vNear), glm::vec3(1.f, 0, 0)));

		y_planes[y] = p;
	}

	//static const std::array<float, zResolution + 1> z_planes = { { 0.1, 0.17, 0.29, 0.49, 0.84, 1.43, 2.44, 4.15, 7.07, 12.0, 20.5, 34.9, 59, 101, 172, 293, 2000 } };
	for (auto z = 0; z <= zRes; ++z)
	{
		float sz = (float(z) / float(zRes)) * 2.f - 1.f;
		auto vZ = inverseProjection * glm::vec4(0, 0, sz, 1.f);
		vZ /= vZ.w;

		Plane p;
		p.pointOnPlane = glm::vec3(0, 0, vZ.z);
		p.normal = glm::vec3(0.f, 0.f, 1.f);

		z_planes[z] = p;
	}
}	

void MassiveLightingClusterStage::createClusters(const GPULights & lights)
{
	auto xRes = xResolution.data(), yRes = yResolution.data(), zRes = zResolution.data();

	// Clear clusters
	for (int i = 0; i < xRes * yRes * zRes; ++i)
		m_lightCounts[i] = 0;

	auto viewMatrix = camera.data()->view();

	for (unsigned i = 0; i < lights.number_of_lights; ++i)
	{
		GPULight light = lights.lights[i];

		// Determine position of light in camera space
		glm::vec4 lightPositionWorld = light.position;
		lightPositionWorld.w = 1.0f;
		glm::vec4 lightPositionView = viewMatrix * lightPositionWorld;
		lightPositionView /= lightPositionView.w;

		float influenceRadius = m_lightRadiuses[i];

		// Determine center_z
		int center_z = std::find_if(z_planes.begin(), z_planes.end(),
			[&](Plane & z_plane) { return lightPositionView.z > z_plane.pointOnPlane.z; }) 
			- z_planes.begin() - 1;

		// Determine center_y
		auto y_light_screen = projection.data()->projection() * lightPositionView;
		y_light_screen /= y_light_screen.w;
		int center_y = int((y_light_screen.y + 1.f) / 2.f * float(yRes));

		for (int z = 0; z < zRes; ++z)
		{
			glm::vec3 z_light(lightPositionView);
			
			if (z != center_z)
			{
				auto & z_plane = (z < center_z) ? z_planes[z + 1] : z_planes[z];
				z_light = projectToZPlane(z_light, z_plane);
			}

			float shortestDistanceToZPlane = glm::length((z_light - glm::vec3(lightPositionView)));
			if (shortestDistanceToZPlane > influenceRadius)
				continue;

			for (int y = 0; y < yRes; ++y)
			{
				glm::vec3 y_light = z_light;
				if (y != center_y)
				{
					auto & y_plane = (y < center_y) ? y_planes[y + 1] : y_planes[y];
					y_light = projectToYPlane(y_light, y_plane);
				}

				float shortestDistanceToYPlane = glm::length((y_light - glm::vec3(lightPositionView)));
				if (shortestDistanceToYPlane > influenceRadius)
					continue;

				float yz_light_radius = sqrt(pow(influenceRadius, 2) - pow(shortestDistanceToYPlane, 2));

				int xLower = 0;
				do
				{
					++xLower;
				} while (xLower < xRes && getDistance(y_light, x_planes[xLower]) >= yz_light_radius);

				int xUpper = xRes;
				do
				{
					--xUpper;
				} while (xUpper >= xLower && -getDistance(y_light, x_planes[xUpper]) >= yz_light_radius);

				for (--xLower; xLower <= xUpper; ++xLower)
				{
					auto clusterIdx = xLower + y * xRes + z * xRes * yRes;
					auto & n = m_lightCounts[clusterIdx];
					if (n < m_cluster[clusterIdx].size())
					{
						m_cluster[clusterIdx][n++] = i;
					}
					else
					{
						//globjects::info() << "light cluster filled";
					}
				}
			}
		}
	}

	// Create index list and 3d texture data of clusters
    
	size_t totalLights = 0;
	for (int i = 0; i < xRes * yRes * zRes; ++i)
		totalLights += m_lightCounts[i];

	auto height = totalLights / m_lookupTextureWidth;
	if (totalLights % m_lookupTextureWidth != 0) ++height;
	m_indices.resize(m_lookupTextureWidth * height);

	int currentOffset = 0;
    for (auto z = 0; z < zRes; ++z)
    {
        for (auto y = 0; y < yRes; ++y)
        {
            for (auto x = 0; x < xRes; ++x)
            {
				auto clusterIdx = x + y * xRes + z * xRes * yRes;
				int count = m_lightCounts[clusterIdx];
				m_lookUp[clusterIdx] = glm::ivec2(currentOffset, count);
				
				for (auto n = 0; n < count; ++n)
                {
					m_indices[currentOffset + n] = m_cluster[clusterIdx][n];
                }

				currentOffset += count;
            }
        }
    }
}

glm::vec3 MassiveLightingClusterStage::projectToZPlane(const glm::vec3& point, const Plane& plane)
{
	auto result = point;
	result.z = plane.pointOnPlane.z;
	return result;
}

glm::vec3 MassiveLightingClusterStage::projectToYPlane(const glm::vec3 & point, const Plane& plane)
{
	auto result = point;
	result.y = (plane.pointOnPlane * point.z).y;
	return result;
}

float MassiveLightingClusterStage::getDistance(const glm::vec3 & point, const Plane &plane)
{
	return point.x - (plane.pointOnPlane * point.z).x;
}