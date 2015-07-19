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
}

void MassiveLightingClusterStage::initialize()
{
	auto texture = globjects::make_ref<globjects::Texture>(GL_TEXTURE_3D);
    texture->setName("cluster Texture");
    texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    texture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	clusterTexture.setData(texture);

    texture = globjects::make_ref<globjects::Texture>(GL_TEXTURE_1D);
    texture->setName("light indices Texture");
    texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    texture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	
    lightIndicesTexture.setData(texture);

	m_indices.reserve(2048);
}


void MassiveLightingClusterStage::updateLightRadiuses()
{
	m_lightRadiuses.clear();
	m_lightRadiuses.reserve(gpuLights.data().number_of_lights);
	for (auto & gpuLight : gpuLights.data().lights)
	{
		// Calculate distance where attenuation(d) = epsilon
		static const float attenuation_epsilon = 0.005f;
		// http://gamedev.stackexchange.com/questions/56897/glsl-light-attenuation-color-and-intensity-formula

		// Point light
		float linearAttenuation = gpuLight.attenuation.y;
		float quadricAttenuation = gpuLight.attenuation.z;
		
		float distLinear = linearAttenuation > 0.f ? 1.f / linearAttenuation : std::numeric_limits<float>::infinity();
		float distQudratic = quadricAttenuation > 0.f ? sqrt(1.f / (quadricAttenuation * attenuation_epsilon)) : std::numeric_limits<float>::infinity();
		m_lightRadiuses.push_back(std::min(distLinear, distQudratic));
	}
}

void MassiveLightingClusterStage::process()
{
    bool recluster = false;

    if (camera.hasChanged())
    {
        recluster = true;
    }

	if (gpuLights.hasChanged())
	{
		updateLightRadiuses();
	}

    if (recluster)
    {
        createCluster();

		clusterTexture.data()->image3D(0, GL_RG32I, xResolution, yResolution, zResolution, 0, GL_RG_INTEGER, GL_INT, m_lookUp);
		lightIndicesTexture.data()->image1D(0, GL_R16UI, m_indices.size(), 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, m_indices.data());
    }	
}

float MassiveLightingClusterStage::z_planes(unsigned i)
{
	//static const std::array<float, zResolution + 1> z_planes = { { 0.1, 0.17, 0.29, 0.49, 0.84, 1.43, 2.44, 4.15, 7.07, 12.0, 20.5, 34.9, 59, 101, 172, 293, 2000 } };
	//return ((z_planes[i] - z_planes[0]) / z_planes[zResolution]) * 2.f - 1.f;
	
	if (i >= zResolution)
		return 1.f;

	return (float(i) / float(zResolution)) * 2.f - 1.f;
}

float MassiveLightingClusterStage::y_planes(unsigned i)
{
	if (i >= yResolution)
		return 1.f;

	return (float(i) / float(yResolution)) * 2.f - 1.f;
}

float MassiveLightingClusterStage::x_planes(unsigned i)
{
	if (i >= xResolution)
		return 1.f;

	return (float(i) / float(xResolution)) * 2.f - 1.f;
}

void MassiveLightingClusterStage::createCluster()
{
	// Clear clusters
	for (int z = 0; z < zResolution; ++z)
	{
		for (int y = 0; y < yResolution; ++y)
		{
			for (int x = 0; x < xResolution; ++x)
			{
				m_lightCounts[x][y][z] = 0;
			}
		}
	}
	m_indices.clear();

	auto viewProjection = projection.data()->projection() * camera.data()->view();

	for (unsigned i = 0; i < gpuLights.data().number_of_lights; ++i)
	{
		//if (i > 0) continue;
		GPULight light = gpuLights.data().lights[i];

		// Determine position of light in screenspace
		glm::vec4 lightPositionWorld = light.position;
		lightPositionWorld.w = 1.0f;
		glm::vec4 lightPositionScreen = viewProjection * lightPositionWorld;
		lightPositionScreen /= lightPositionScreen.w;

		float influenceRadius = m_lightRadiuses[i];
		
		glm::vec4 influenceRadiusWorld = glm::vec4(lightPositionWorld.x, lightPositionWorld.y + influenceRadius, lightPositionWorld.z, 1.f);
		glm::vec4 projectedInfluenceRadius = viewProjection * influenceRadiusWorld;
		projectedInfluenceRadius /= projectedInfluenceRadius.w;
		float theRealInfluenceRadius = glm::length(lightPositionScreen - projectedInfluenceRadius);
		//theRealInfluenceRadius = 0.3f;
		//lightPositionScreen = glm::vec4(0);
		//globjects::info() << theRealInfluenceRadius;
		//globjects::info() << lightPositionScreen;

		for (int z = 0; z < zResolution; ++z)
		{
			glm::vec3 z_light(lightPositionScreen);
			
			
			if (z_light.z < z_planes(z) || z_light.z > z_planes(z+1))
			{
				float z_plane = (z_light.z < z_planes(z)) ? z_planes(z) : z_planes(z + 1);
				z_light.z = z_plane;
			}

			for (int y = 0; y < yResolution; ++y)
			{
				glm::vec3 y_light = z_light;
				if (y_light.y < y_planes(y) || y_light.y > y_planes(y+1))
				{
					float y_plane = (y_light.y < y_planes(y)) ? y_planes(y) : y_planes(y + 1);
					y_light.y = y_plane;
				}

				float t_radius = glm::length((y_light - glm::vec3(lightPositionScreen)));
				if (t_radius > theRealInfluenceRadius)
					continue;

				float yz_light_radius = sqrt(pow(theRealInfluenceRadius, 2) - pow(t_radius, 2));

				int xLower = 0;
				do
				{
					++xLower;
				} while (xLower < xResolution && (y_light.x - x_planes(xLower)) >= yz_light_radius);

				int xUpper = xResolution;
				do
				{
					--xUpper;
				} while (xUpper >= xLower && -(y_light.x - x_planes(xUpper)) >= yz_light_radius);

				for (--xLower; xLower <= xUpper; ++xLower)
				{
					auto & n = m_lightCounts[xLower][y][z];
					if (n < m_cluster[xLower][y][z].size())
					{
						m_cluster[xLower][y][z][n++] = i;
					}
					else
					{
						//globjects::info() << "ich bin light #" << i;
						//n = 5;
					}
				}
			}
		}
	}

	// Create index list and 3d texture data of clusters
    int currentOffset = 0;
    for (int z = 0; z < zResolution; ++z)
    {
        for (int y = 0; y < yResolution; ++y)
        {
            for (int x = 0; x < xResolution; ++x)
            {
				int count = m_lightCounts[x][y][z];
				m_lookUp[x + y * xResolution + z * xResolution * yResolution] = glm::ivec2(currentOffset, count);
				currentOffset += count;
				
				for (auto n = 0; n < m_lightCounts[x][y][z]; ++n)
                {
					m_indices.push_back(m_cluster[x][y][z][n]);
                }
            }
        }
    }
}
