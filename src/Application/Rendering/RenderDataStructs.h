#pragma once

#include <glm/gtx/common.hpp>

// 64 bytes
struct Light
{
	Light() :
		position(0.0f),
		type(0), color(0.0f),
		amountOfLights(0),
		cutoff(0.0f),
		constant(0.0f), linear(0.0f), quadratic(0.0f)
	{

	}
	// 16 bytes
	glm::vec3 position; // either position or direction
	uint32_t type;

	// 16 bytes
	glm::vec3 color;
	int amountOfLights;

	// 16 bytes
	float constant;
	float linear;
	float quadratic;
	float cutoff;

	glm::vec4 pad;
};
