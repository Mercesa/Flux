#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <optional>
#include <stdint.h>

namespace Flux
{
	namespace Gfx
	{

		// For now only support for 1 queue per type
		// The boolean will be set to true when used and to false when removed
		struct QueueFamilyIndices {
			std::optional<std::pair<uint32_t, bool>> graphicsFamily;
			std::optional<std::pair<uint32_t, bool>> presentFamily;
			std::optional<std::pair<uint32_t, bool>> transferFamily;
			std::optional<std::pair<uint32_t, bool>> computeFamily;


			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value() && computeFamily.has_value();
			}
		};

		struct QueueData;

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

