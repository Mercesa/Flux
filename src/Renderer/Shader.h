#pragma once

#include "vulkan/vulkan.h"
#include "Renderer/ShaderReflection.h"

#include "Renderer/GraphicEnums.h"

#include <string>

namespace Flux
{
	namespace Gfx
	{
		inline VkShaderStageFlagBits ConvertShaderToVkShaderStageBit(Flux::Gfx::ShaderTypes aType)
		{
			switch (aType)
			{
			case Flux::Gfx::ShaderTypes::eVertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
				break;
			case Flux::Gfx::ShaderTypes::eFragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			case Flux::Gfx::ShaderTypes::eGeometry:
				return VK_SHADER_STAGE_GEOMETRY_BIT;
				break;
			case Flux::Gfx::ShaderTypes::eTessellationControl:
				return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				break;
			case Flux::Gfx::ShaderTypes::eTessellationEval:
				return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				break;
			case Flux::Gfx::ShaderTypes::eCompute:
				return VK_SHADER_STAGE_COMPUTE_BIT;
				break;
			case Flux::Gfx::ShaderTypes::eRayGen:
				return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
				break;
			case Flux::Gfx::ShaderTypes::eRayMiss:
				return VK_SHADER_STAGE_MISS_BIT_KHR;
				break;
			case Flux::Gfx::ShaderTypes::eRayClosestHit:
				return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
				break;
			case Flux::Gfx::ShaderTypes::eRayAnyHit:
				return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
				break;
			case Flux::Gfx::ShaderTypes::eRayIntersection:
				return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
				break;
			case Flux::Gfx::ShaderTypes::eRayCallable:
				return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
				break;

			default:
				VkShaderStageFlagBits(0);
			}
		}

		struct ShaderCreateDesc
		{
			std::string mFilePath;
			ShaderTypes mType;
			std::vector<char> mCode;
		};

		struct Shader
		{
			Shader() : mShadertype(ShaderTypes::eUnknownShaderType), mFilePath(""), mReflectionData(), mShaderModule(VK_NULL_HANDLE)
			{

			}
			// General code
			ShaderTypes mShadertype;
			std::string mFilePath;
			ShaderReflection::ShaderReflectionData mReflectionData;

			// Vulkan
			VkShaderModule mShaderModule;
		};
	}
}