#pragma once

#include "vulkan/vulkan.h"
#include "Renderer/ShaderReflection.h"

#include <string>

namespace Flux
{
	namespace Gfx
	{
		enum class ShaderTypes
		{
			eVertex,
			eFragment,
			eGeometry,
			eTessellationControl,
			eTessellationEval,
			eCompute,
			eRayGen,
			eRayClosestHit,
			eRayMiss,
			eRayAnyHit,
			eRayIntersection,
			eRayCallable
		};

		struct ShaderCreateDesc
		{
			std::string mFilePath;
			ShaderTypes mType;
		};

		struct Shader
		{
			// General code
			ShaderTypes mShadertype;
			std::string mEntryPoint;
			std::string mFilePath;
			ShaderReflection::ShaderResourceReflection mReflectionData;

			// Vulkan
			VkShaderModule mShaderModule;
		};
	}
}