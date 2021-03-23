#pragma once

#include <cassert>
#include <vector>
#include <string>

#include "vulkan/vulkan.h"


namespace Flux
{
	namespace Gfx
	{
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
				eSeparateSamplers
			};

			struct ShaderResourceReflection
			{
				int32_t mBindingNumber;
				int32_t mSetNumber;
				std::string mName;
				ShaderResourceType mType;
			};

			struct ShaderReflection
			{
				std::vector<ShaderResourceReflection> mResources;
				std::string mEntryPoint;
				uint32_t mThreadGroups[3]; // X Y Z
			};

			ShaderReflection Reflect(std::vector<char> aSpvbinary);
		}
	}
}