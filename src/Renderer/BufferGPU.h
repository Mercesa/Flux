#pragma once
#include <vulkan/vulkan.h>
#include <VmaUsage.h>

namespace Flux
{
	namespace Gfx
	{


	class BufferGPU
	{
	public:
		VkBuffer mBuffer;
		VkBufferUsageFlags mUsageFlags;
		VmaAllocation mAllocation;
		VmaMemoryUsage mMemoryUsage;
	};
	}
}