#include "MassiveLightingDemoLightStage.h"

#include <random>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.inl>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Texture.h>
#include <globjects/Logging.h>

#include <gloperate/primitives/Light.h>
using namespace gl;

MassiveLightingDemoLightStage::MassiveLightingDemoLightStage()
	: AbstractStage("MassiveLightingDemoLightStage")
{
	addInput("time", time);
	addInput("activeLights", activeLights);
	addInput("animateLights", animateLights);
}
float getRandomFloat(float lowerBound, float upperBound)
{
    return lowerBound + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(upperBound-lowerBound)));
}

void MassiveLightingDemoLightStage::initialize()
{
	m_lightInitialTransform.resize(MAX_LIGHTS);

	GPULights lights;
	lights.ambient_color = glm::vec4(1, 1, 1, 1);
    const float maxRadius = 20.0f;

	for (auto i = 0; i < MAX_LIGHTS; ++i)
	{
		auto & light = lights.lights[i];
		auto & initialTransform = m_lightInitialTransform[i];

        initialTransform.radius = std::sqrt(getRandomFloat(0.0f, 1.0f)) * maxRadius;
        initialTransform.angle = getRandomFloat(0.0f, 2.0f * glm::pi<float>());
        initialTransform.height = getRandomFloat(0.0f, 5.0f);
		// Normalize by arc length
        initialTransform.animationSpeed = getRandomFloat(2.0f, 20.0f) / initialTransform.radius;
		light.position.w = static_cast<int>(gloperate::LightSourceType::Point);
        light.attenuation = glm::vec4(0, 3, 0, 0);
        light.color = glm::vec4(getRandomFloat(0.1f, 0.5f) * hueToRgb(getRandomFloat(0.1f, 0.5f)), 1.f);
	}

	gpuLights.setData(lights);
}

void MassiveLightingDemoLightStage::process()
{
	if (activeLights.hasChanged())
		gpuLights.data().number_of_lights = activeLights.data();

	if ((animateLights.hasChanged() || time.hasChanged() || activeLights.hasChanged()) && animateLights.data())
	{
		gpuLights.data().number_of_lights = activeLights.data();

		for (auto i = 0; i < activeLights.data(); ++i)
		{
			auto & light = gpuLights.data().lights[i];
			const auto & initialTransform = m_lightInitialTransform[i];
			auto angle = initialTransform.angle + time.data()->time() * initialTransform.animationSpeed;
			light.position = glm::vec4(
				initialTransform.radius * std::cos(angle),
				initialTransform.height,
				initialTransform.radius * std::sin(angle),
				1.f);
		}

		gpuLights.invalidate();
	}
}

glm::vec3 MassiveLightingDemoLightStage::hueToRgb(float hue)
{
    double intPart;
	float fracPart = modf(hue * 6.0f, &intPart);
	int region = static_cast<int>(intPart);

	switch (region) {
	case 0: return glm::vec3(1.0f, fracPart, 0.0f);
	case 1: return glm::vec3(1.0f - fracPart, 1.0f, 0.0f);
	case 2: return glm::vec3(0.0f, 1.0f, fracPart);
	case 3: return glm::vec3(0.0f, 1.0f - fracPart, 1.0f);
	case 4: return glm::vec3(fracPart, 0.0f, 1.0f);
	case 5: return glm::vec3(1.0f, 0.0f, 1.0f - fracPart);
	};

	return glm::vec3(0.0f, 0.0f, 0.0f);
}
