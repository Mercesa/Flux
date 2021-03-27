#pragma once

#include "vulkan/vulkan.h"
#include "Renderer/ShaderReflection.h"

#include "Renderer/GraphicEnums.h"

#include <string>

namespace Flux
{
	namespace Gfx
	{

		struct ShaderCreateDesc
		{
			std::string mFilePath;
			ShaderTypes mType;
		};

		struct Shader
		{
			Shader() : mShadertype(ShaderTypes::eUnknownShaderType), mEntryPoint(""), mFilePath("")
			{

			}
			// General code
			ShaderTypes mShadertype;
			std::string mEntryPoint;
			std::string mFilePath;
			ShaderReflection::ShaderReflectionData mReflectionData;

			// Vulkan
			VkShaderModule mShaderModule;
		};
	}
}