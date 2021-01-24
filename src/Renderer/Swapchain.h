#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <VmaUsage.h>

namespace Flux
{
	namespace Gfx
	{
		class Swapchain
		{
		public:
			VkSwapchainKHR mSwapChain;
			std::vector<VkImage> mImages;
			std::vector<VkImageView> mImageViews;
			VkFormat mImageFormat;
			VkExtent2D mExtent;

			VkImage depthImage;
			VmaAllocation depthImageMemory;
			VkImageView depthImageView;

			std::vector<VkFramebuffer> mFramebuffers;
		};
	}
}
