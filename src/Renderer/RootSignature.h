#pragma once

#include <vector>
#include "vulkan/vulkan.h"

#include "Renderer/Shader.h"
#include "Renderer/ShaderReflection.h"

namespace Flux
{
	namespace Gfx
	{
		struct RootSignatureCreateDesc
		{
			std::vector<std::shared_ptr<Shader>> mShaders;
		};

		struct RootSignature
		{
			std::vector<std::shared_ptr<Shader>> mShaders;
			std::vector<Flux::Gfx::ShaderReflection::ShaderResourceReflection> mRootSignatureResources; // Collection of all the usable shader resources from all the shaders within this root signature
			std::vector<Flux::Gfx::ShaderReflection::PushConstantReflection> mPushConstantBuffers;

			// Vulkan
			VkPipelineLayout mPipelineLayout;
			std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
		};
	}
}