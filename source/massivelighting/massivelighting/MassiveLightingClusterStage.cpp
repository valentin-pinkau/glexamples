#include "MassiveLightingClusterStage.h"

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <globjects/Texture.h>

MassiveLightingClusterStage::MassiveLightingClusterStage()
:   AbstractStage("MassiveLightingClusterStage")
{

}

void MassiveLightingClusterStage::initialize()
{
    m_clusterTexture = new globjects::Texture(gl::GL_TEXTURE_3D);
    m_clusterTexture->setName("cluster Texture");
    m_clusterTexture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    m_clusterTexture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    m_clusterTexture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    m_clusterTexture->setParameter(gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    m_clusterTexture->setParameter(gl::GL_TEXTURE_WRAP_R, gl::GL_CLAMP_TO_EDGE);
}


void MassiveLightingClusterStage::process()
{
    bool recluster = false;

    if(camera.hasChanged())
    {
        recluster = true;
    }

    if(recluster)
    {
        createCluster();
    }
}

void MassiveLightingClusterStage::createCluster()
{
    const float attenuation_epsilon = 0.01f;
    for (int i = 0; i < gpuLights.data().number_of_lights; ++i)
    {
        GPULight light = gpuLights.data().lights[i];
        glm::vec4 position = light.position;
        position.w = 1.0f;
        glm::vec4 lightposition = projection.data()->projection() * camera.data()->view() * position;
        lightposition /= lightposition.w;
        float linAtt = light.attenuation.y;
        float quadAtt = light.attenuation.z;
        float pHalf = (linAtt / (quadAtt * 2.0f));
        float influenceRadius = abs(- pHalf + sqrt(pHalf * pHalf - (1.0 / attenuation_epsilon * quadAtt)));
        glm::vec3 llf = glm::vec3(position.x - influenceRadius, position.y - influenceRadius, position.z - influenceRadius);
        glm::vec3 urb = glm::vec3(position.x + influenceRadius, position.y + influenceRadius, position.z + influenceRadius);
        glm::vec3 llfClusterIdx(floor(llf.x * xResolution), floor(llf.y * yResolution), floor(llf.z * zResolution));
        glm::vec3 urbClusterIdx(floor(urb.x * xResolution), floor(urb.y * yResolution), floor(urb.z * zResolution));

        for (int z = llfClusterIdx.z; z <= urbClusterIdx.z; ++z)
        {
            for (int y = llfClusterIdx.y; y <= urbClusterIdx.y; ++y)
            {
                for (int x = llfClusterIdx.x; x <= urbClusterIdx.x; ++x)
                {
                    m_cluster[x][y][z].push_back(i);
                }
            }
        }
    }

}

