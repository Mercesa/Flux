#pragma once
#include <vector>
#include "vulkan/vulkan.h"


class SwapchainVK
{
public:
	VkFormat swapChainImageFormat;
	VkSwapchainKHR swapChain;
	VkExtent2D swapChainExtent;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
};

