#pragma once

#include "vulkan/vulkan.h"
#include "VmaUsage.h"
#include "GraphicsDevice.h"

namespace Flux
{
	namespace Gfx
	{
		struct RenderContext
		{
			VkInstance instance;
			VmaAllocator memoryAllocator;
			std::string name;
			std::shared_ptr<GraphicsDevice> mDevice;
			bool debugMode = true;
			VkDebugUtilsMessengerEXT debugMessenger;
			VkSurfaceKHR surface;
			bool vsync = true;

			VkDebugReportCallbackEXT        pVkDebugReport;

		};
	}
}