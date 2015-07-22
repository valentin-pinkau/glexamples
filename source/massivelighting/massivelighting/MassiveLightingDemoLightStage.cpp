#include "MassiveLightingDemoLightStage.h"

#include <random>

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

void MassiveLightingDemoLightStage::initialize()
{
	m_lightInitialTransform.resize(MAX_LIGHTS);

	GPULights lights;
	lights.ambient_color = glm::vec4(1, 1, 1, 1);

	std::mt19937 rng(1337);

	std::tr1::uniform_real<float> radiusNormDist(0.0f, 1.0f);
	const float maxRadius = 20.0f;
	std::uniform_real<float> angleDist(0.0f, 2.0f * glm::pi<float>());
	std::uniform_real<float> heightDist(0.0f, 5.0f);
	std::uniform_real<float> animationSpeedDist(2.0f, 20.0f);
	std::uniform_int<int> animationDirection(0, 1);
	std::uniform_real<float> hueDist(0.0f, 1.0f);
	std::uniform_real<float> intensityDist(0.1f, 0.5f);

	for (auto i = 0; i < MAX_LIGHTS; ++i)
	{
		auto & light = lights.lights[i];
		auto & initialTransform = m_lightInitialTransform[i];

		initialTransform.radius = std::sqrt(radiusNormDist(rng)) * maxRadius;
		initialTransform.angle = angleDist(rng);
		initialTransform.height = heightDist(rng);
		// Normalize by arc length
		initialTransform.animationSpeed = (animationDirection(rng) * 2 - 1) * animationSpeedDist(rng) / initialTransform.radius;

		light.position.w = static_cast<int>(gloperate::LightSourceType::Point);
		light.attenuation = glm::vec4(0, 2, 0, 0);
		light.color = glm::vec4(intensityDist(rng) * hueToRgb(hueDist(rng)), 1.f);
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
	float intPart;
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