#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <optional>
#include <stdint.h>

namespace Flux
{
	namespace Gfx
	{

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct GraphicsDevice
		{
			VkPhysicalDevice mPhysicalDevice;
			VkPhysicalDeviceFeatures2 mPhysicalDeviceFeatures;
			VkPhysicalDeviceMemoryProperties mPhysicalDeviceMemoryProperties;
			VkDevice mDevice;
			std::string mDeviceName;
			QueueFamilyIndices queueFamilies;
		};
	}

}

