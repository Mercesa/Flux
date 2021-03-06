#pragma once

#include <vulkan/vulkan.h>
#include <VmaUsage.h>
#include <vector>

#include "TextureVK.h"

namespace Flux
{
	namespace Gfx
	{

		struct TargetDesc
		{
			VkFormat format;
		};

		struct RenderTargetCreateDesc
		{
			uint32_t mWidth;
			uint32_t mHeight;
			std::vector<TargetDesc> mTargets;
			std::optional<TargetDesc>	mDepthTarget;
		};

		struct RenderTarget
		{

			// Array of textures
			// Single depth texture
			// Renderpass
			// array Framebuffer
			uint32_t mWidth;
			uint32_t mHeight;
			std::vector<std::shared_ptr<Gfx::Texture>> mColorImages;
			std::shared_ptr<Gfx::Texture> mDepthImage;

			VkFramebuffer mFramebuffer;
			VkRenderPass mPass;

			//,
			//	VkFramebuffer aFrameBuffer, VkRenderPass aRenderPass,
			//	std::shared_ptr<Gfx::Texture> aDepthImage = nullptr

			RenderTarget(
				uint32_t aWidth, uint32_t aHeight,
				std::vector<std::shared_ptr<Gfx::Texture>> aColorImages) :
				mWidth(aWidth), mHeight(aHeight),
				mColorImages(mColorImages)
			{}
		};
	}
}