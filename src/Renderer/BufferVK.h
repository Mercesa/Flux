#pragma once
#include "vulkan/vulkan.h"
#include <VmaUsage.h>

namespace Flux
{
	class BufferVK
	{
	public:
		VkBuffer mBuffer;
		VkBufferUsageFlags mUsageFlags;
		VmaAllocation mAllocation;
		VmaMemoryUsage mMemoryUsage;
	};
}