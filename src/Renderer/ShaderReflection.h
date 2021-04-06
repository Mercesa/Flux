#pragma once

#include <cassert>
#include <vector>
#include <string>
#include <memory>

#include "vulkan/vulkan.h"

#include "GraphicEnums.h"

namespace Flux
{
	namespace Gfx
	{
		struct Shader;

		namespace ShaderReflection
		{
			enum class ShaderResourceType
			{
				eUniform_buffer,
				eStorage_buffer,
				eStage_input,
				eStage_output,
				eSubpass_inputs,
				eStorage_images,
				eSampled_images,
				eAtomic_counters,
				ePushConstantBuffer,
				eAccelerationStructure,
				eSeparateImages,
				eSeparateSamplers,
				eUnknownResource
			};

			struct ShaderResourceReflection
			{
				ShaderResourceReflection() : mBindingNumber(-1), mSetNumber(-1), mSize(0), mName(""), mType(ShaderResourceType::eUnknownResource), mShaderAccess(ShaderTypes::eUnknownShaderType){}

				int32_t mBindingNumber;
				int32_t mSetNumber;
				uint32_t mSize;
				std::string mName;
				ShaderResourceType mType;
				uint32_t mShaderAccess;
			};

			struct PushConstantReflection
			{
				int32_t mSize;
				uint32_t mShaderAccess;
			};

			struct ShaderReflectionData
			{
				std::vector<ShaderResourceReflection> mResources;
				std::vector<PushConstantReflection> mPushConstants;
				std::string mEntryPoint;
				uint32_t mThreadGroups[3]; // X Y Z
			};

			ShaderReflectionData Reflect(std::vector<char> aSpvbinary);

			// Validates a bunch of shaders to see if their resources are overlapping and not causing conflicts
			std::vector<ShaderResourceReflection> ValidateAndMergeShaderResources(const std::vector<std::shared_ptr<Flux::Gfx::Shader>> aShaders);
			std::vector<PushConstantReflection> MergeRootConstants(const std::vector<std::shared_ptr<Flux::Gfx::Shader>> aShaders);

		}
	}
}