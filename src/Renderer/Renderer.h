#pragma once

#include <stdexcept>
#include <cassert>
#include <vector>

#include "vulkan/vulkan.h"

namespace Flux
{
	class Renderer
	{
    public:
		VkShaderModule CreateShaderModule(VkDevice aDevice, const std::vector<char>& code);

		VkCommandBuffer BeginSingleTimeCommands(VkDevice aDevice, VkCommandPool aCmdPool);

		void EndSingleTimeCommands(VkDevice aDevice, VkQueue aQueue, VkCommandBuffer aCommandBuffer, VkCommandPool aCmdPool);
	};
};
