#pragma once
#include <vulkan/vulkan.h>
#include <VmaUsage.h>

namespace Flux
{
	namespace Gfx
	{
		class Texture
		{
		public:
			VkImage mImage;
			VkImageView mView;
			VkFormat mFormat;
			VmaAllocation mAllocation;
		};
	}

}

