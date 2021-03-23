#pragma once

#include <vector>
#include "vulkan/vulkan.h"
#include "Renderer/Shader.h"

namespace Flux
{
	namespace Gfx
	{
		struct RootSignatureCreateDesc
		{
			std::vector<Shader> mShaders;
		};

		struct RootSignature
		{
			std::vector<Shader> mShaders;

			// Vulkan
			VkPipelineLayout pipelineLayout;
			std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
		};
	}
}