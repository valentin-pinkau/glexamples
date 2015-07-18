#include "MassiveLightingClusterStage.h"

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>

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


void MassiveLightingClusterStage::process()
{
    bool recluster = false;

    if (camera.hasChanged())
    {
        recluster = true;
    }

    if (recluster)
    {
        createCluster();

		clusterTexture.data()->image3D(0, GL_RG32I, xResolution, yResolution, zResolution, 0, GL_RG_INTEGER, GL_INT, m_lookUp);
        lightIndicesTexture.data()->image1D(0, GL_R16I, m_indices.size(), 0, GL_RED_INTEGER, GL_INT, m_indices.data());
    }	
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

    static const float attenuation_epsilon = 0.01f;
	auto viewProjection = projection.data()->projection() * camera.data()->view();
    for (unsigned i = 0; i < gpuLights.data().number_of_lights; ++i)
    {
        GPULight light = gpuLights.data().lights[i];
        
		// Determine position of light in screenspace
		glm::vec4 lightPositionWorld = light.position;
        lightPositionWorld.w = 1.0f;
        glm::vec4 lightPositionScreen =  viewProjection * lightPositionWorld;
        lightPositionScreen /= lightPositionScreen.w;

		// Calculate distance where attenuation(d) = epsilon
		float linearAttenuation = 1; // light.attenuation.y;
		float quadricAttenuation = 0.1f; // light.attenuation.z;
		if (quadricAttenuation < 0.001f) quadricAttenuation += 0.1;
        float pHalf = (linearAttenuation / (quadricAttenuation * 2.0f));
        float influenceRadius = abs(- pHalf + sqrt(pHalf * pHalf - (1.0 / attenuation_epsilon * quadricAttenuation)));

		glm::vec4 influenceRadiusWorld = glm::vec4(lightPositionWorld.x + influenceRadius, lightPositionWorld.y, lightPositionWorld.z, 1.f);
		glm::vec4 projectedInfluenceRadius = viewProjection * influenceRadiusWorld;
		float dist = glm::length(lightPositionScreen - projectedInfluenceRadius);

		// Create AABB of light using the calculated distance
		glm::vec4 llf = glm::vec4(lightPositionScreen.x - dist, 
									   lightPositionScreen.y - dist, 
									   lightPositionScreen.z - dist, 1.0f);
		glm::vec4 urb = glm::vec4(lightPositionScreen.x + dist, 
								       lightPositionScreen.y + dist, 
								       lightPositionScreen.z + dist, 1.0f);

		// Determine cluster coordinates of lower left front / upper right back
		// vertices of AABB
        glm::ivec3 llfClusterIdx(floor(llf.x * xResolution), floor(llf.y * yResolution), floor(llf.z * zResolution));
        glm::ivec3 urbClusterIdx(floor(urb.x * xResolution), floor(urb.y * yResolution), floor(urb.z * zResolution));
		
		// Assign light to clusters
        for (int z = llfClusterIdx.z; z <= urbClusterIdx.z; ++z)
        {
            for (int y = llfClusterIdx.y; y <= urbClusterIdx.y; ++y)
            {
                for (int x = llfClusterIdx.x; x <= urbClusterIdx.x; ++x)
                {
					// If current cluster is not visible, skip it
					if (x < 0 || y < 0 || z < 0 ||
						x >= xResolution || y >= yResolution || z >= zResolution)
						continue;

					auto n = m_lightCounts[x][y][z]++;
					m_cluster[x][y][z][n] = i;
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
