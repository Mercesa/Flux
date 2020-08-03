#pragma once
#include <vector>
#include <vulkan/vulkan.h>


namespace Flux
{


	class SwapchainVK
	{
	public:
		VkSwapchainKHR mSwapChain;
		std::vector<VkImage> mImages;
		VkFormat mImageFormat;
		VkExtent2D mExtent;;
		std::vector<VkImageView> mImageViews;
		std::vector<VkFramebuffer> mFramebuffers;
	};
};
