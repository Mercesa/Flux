#pragma once
#include <vulkan/vulkan.h>
#include <VmaUsage.h>

namespace Flux
{
	class TextureVK
	{
	public:
		VkImage mImage;
		VkImageView mView;
		VkFormat mFormat;
		VmaAllocation mAllocation;
	};
}

