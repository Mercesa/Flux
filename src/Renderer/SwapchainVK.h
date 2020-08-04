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
		std::vector<VkImageView> mImageViews;
		VkFormat mImageFormat;
		VkExtent2D mExtent;
		
		VkImageView mDepthViews;
		VkImage mDepthImage;

		std::vector<VkFramebuffer> mFramebuffers;
	};
};
