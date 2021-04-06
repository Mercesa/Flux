#include "ShaderReflection.h"


#include <algorithm>

#include "spirv_reflect.hpp"
#include "Shader.h"

using namespace Flux::Gfx::ShaderReflection;
using namespace Flux::Gfx;



enum ExecutionModel {
	ExecutionModelVertex = 0,
	ExecutionModelTessellationControl = 1,
	ExecutionModelTessellationEvaluation = 2,
	ExecutionModelGeometry = 3,
	ExecutionModelFragment = 4,
	ExecutionModelGLCompute = 5,
	ExecutionModelKernel = 6,
	ExecutionModelTaskNV = 5267,
	ExecutionModelMeshNV = 5268,
	ExecutionModelRayGenerationKHR = 5313,
	ExecutionModelRayGenerationNV = 5313,
	ExecutionModelIntersectionKHR = 5314,
	ExecutionModelIntersectionNV = 5314,
	ExecutionModelAnyHitKHR = 5315,
	ExecutionModelAnyHitNV = 5315,
	ExecutionModelClosestHitKHR = 5316,
	ExecutionModelClosestHitNV = 5316,
	ExecutionModelMissKHR = 5317,
	ExecutionModelMissNV = 5317,
	ExecutionModelCallableKHR = 5318,
	ExecutionModelCallableNV = 5318,
	ExecutionModelMax = 0x7fffffff,
};

ShaderTypes ConvertExecutionModelToShaderType(spv::ExecutionModel aExecutionModel)
{
	switch (aExecutionModel)
	{
	case ExecutionModelVertex:
		return ShaderTypes::eVertex;
		break;
	case ExecutionModelTessellationControl:
		return ShaderTypes::eTessellationControl;
		break;
	case ExecutionModelTessellationEvaluation:
		return ShaderTypes::eTessellationEval;
		break;
	case ExecutionModelGeometry:
		return ShaderTypes::eGeometry;
		break;
	case ExecutionModelFragment:
		return ShaderTypes::eFragment;
		break;
	case ExecutionModelGLCompute:
		return ShaderTypes::eCompute;
		break;
	case ExecutionModelRayGenerationKHR:
		return ShaderTypes::eRayGen;
		break;
	case ExecutionModelIntersectionKHR:
		return ShaderTypes::eRayIntersection;
		break;
	case ExecutionModelAnyHitKHR:
		return ShaderTypes::eRayAnyHit;
		break;
	case ExecutionModelClosestHitKHR:
		return ShaderTypes::eRayClosestHit;
		break;
	case ExecutionModelMissKHR:
		return ShaderTypes::eRayMiss;
		break;
	case ExecutionModelCallableKHR:
		return ShaderTypes::eRayCallable;
		break;
	case ExecutionModelMax:
	case ExecutionModelKernel:
	case ExecutionModelTaskNV:
	case ExecutionModelMeshNV:
	default:
		break;
	}
}

std::vector<ShaderResourceReflection> AddResourcesToList(const spirv_cross::CompilerGLSL& aCompiler, const spirv_cross::SmallVector<spirv_cross::Resource>& aResources, ShaderResourceType aType, ShaderTypes aShaderStage)
{
	std::vector<ShaderResourceReflection> tResourceList;
	tResourceList.reserve(aResources.size());

	for (const auto& resource : aResources)
	{
		ShaderResourceReflection tResource{};
		tResource.mSetNumber = aCompiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		tResource.mBindingNumber = aCompiler.get_decoration(resource.id, spv::DecorationBinding);
		tResource.mName = resource.name;
		tResource.mType = aType;
		tResource.mShaderAccess = aShaderStage;


		spirv_cross::SPIRType type = aCompiler.get_type(resource.type_id);


		// Check if the resource is an array
		if (type.array.size())
			tResource.mSize = type.array[0];
		else
			tResource.mSize = 1;

		tResourceList.push_back(tResource);
	}

	return tResourceList;
}

std::vector<PushConstantReflection> AddPushConstant(const spirv_cross::CompilerGLSL& aCompiler, const spirv_cross::SmallVector<spirv_cross::Resource>& aResources, ShaderResourceType aType, ShaderTypes aShaderStage)
{
	std::vector<PushConstantReflection> tPushConstantsVector;

	for (const auto& resource : aResources)
	{
		PushConstantReflection tPushConstants{};
		tPushConstants.mShaderAccess = aShaderStage;

		const spirv_cross::SPIRType& type = aCompiler.get_type(resource.base_type_id);
		tPushConstants.mSize = aCompiler.get_declared_struct_size(type);
		tPushConstantsVector.push_back(tPushConstants);
	}

	return tPushConstantsVector;
}



ShaderReflectionData Flux::Gfx::ShaderReflection::Reflect(std::vector<char> aSpvbinary)
{
	ShaderReflectionData tReflection{};

	spirv_cross::CompilerGLSL glsl((uint32_t*)(aSpvbinary.data()), (aSpvbinary.size() * sizeof(char)) / sizeof(uint32_t));

	tReflection.mEntryPoint = glsl.get_entry_points_and_stages()[0].name;
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

	uint32_t tResourcesSize = 0;

	tResourcesSize += resources.uniform_buffers.size();
	tResourcesSize += resources.storage_buffers.size();
	tResourcesSize += resources.stage_inputs.size();
	tResourcesSize += resources.stage_outputs.size();
	tResourcesSize += resources.subpass_inputs.size();
	tResourcesSize += resources.storage_images.size();
	tResourcesSize += resources.sampled_images.size();
	tResourcesSize += resources.atomic_counters.size();
	tResourcesSize += resources.push_constant_buffers.size();
	tResourcesSize += resources.acceleration_structures.size();
	tResourcesSize += resources.separate_images.size();
	tResourcesSize += resources.separate_samplers.size();

	tReflection.mResources.reserve(tResourcesSize);

	// Query the shader type from the spirv-cross binaries
	ShaderTypes tShaderStage = ConvertExecutionModelToShaderType(glsl.get_entry_points_and_stages()[0].execution_model);

	// Uniform buffers
	{
		auto tUniformBuffers = AddResourcesToList(glsl, resources.uniform_buffers, ShaderResourceType::eUniform_buffer, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tUniformBuffers.begin(), tUniformBuffers.end());
	}

	// Storage buffers
	{
		auto tStorageBuffer = AddResourcesToList(glsl, resources.storage_buffers, ShaderResourceType::eStorage_buffer, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tStorageBuffer.begin(), tStorageBuffer.end());
	}

	// Stage inputs
	{
		auto tStageInputs = AddResourcesToList(glsl, resources.stage_inputs, ShaderResourceType::eStage_input, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tStageInputs.begin(), tStageInputs.end());
	}

	// Stage outputs
	{
		auto tStageOutputs = AddResourcesToList(glsl, resources.stage_outputs, ShaderResourceType::eStage_output, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tStageOutputs.begin(), tStageOutputs.end());
	}

	// Subpass inputs
	{
		auto tSubpassInputs = AddResourcesToList(glsl, resources.subpass_inputs, ShaderResourceType::eSubpass_inputs, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tSubpassInputs.begin(), tSubpassInputs.end());
	}

	// Storage images
	{
		auto tStorageImages = AddResourcesToList(glsl, resources.storage_images, ShaderResourceType::eStorage_images, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tStorageImages.begin(), tStorageImages.end());
	}

	// Sampled images
	{
		auto tSampledImages = AddResourcesToList(glsl, resources.sampled_images, ShaderResourceType::eSampled_images, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tSampledImages.begin(), tSampledImages.end());
	}

	// Atomic counters
	{
		auto tAtomicCounters = AddResourcesToList(glsl, resources.atomic_counters, ShaderResourceType::eAtomic_counters, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tAtomicCounters.begin(), tAtomicCounters.end());
	}

	// Push constants are an exception
	{
		tReflection.mPushConstants = AddPushConstant(glsl, resources.push_constant_buffers, ShaderResourceType::ePushConstantBuffer, tShaderStage);
	}

	// Acceleration structures
	{
		auto tAccelerationStructures = AddResourcesToList(glsl, resources.acceleration_structures, ShaderResourceType::eAccelerationStructure, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tAccelerationStructures.begin(), tAccelerationStructures.end());
	}

	// Separate image
	{
		auto tSeparateImages = AddResourcesToList(glsl, resources.separate_images, ShaderResourceType::eSeparateImages, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tSeparateImages.begin(), tSeparateImages.end());
	}

	// Separate sampler
	{
		auto tSeparateSamplers = AddResourcesToList(glsl, resources.separate_samplers, ShaderResourceType::eSeparateSamplers, tShaderStage);
		tReflection.mResources.insert(tReflection.mResources.end(), tSeparateSamplers.begin(), tSeparateSamplers.end());
	}

	return tReflection;
}

std::vector<ShaderResourceReflection> Flux::Gfx::ShaderReflection::ValidateAndMergeShaderResources(const std::vector<std::shared_ptr<Flux::Gfx::Shader>> aShaders)
{
	if (aShaders.size() == 0)
	{
		return std::vector<ShaderResourceReflection>();
	}

	std::vector<ShaderResourceReflection> tShaderResources;

	const std::vector<ShaderResourceType> exclusions = {
		ShaderResourceType::eStage_input,
		ShaderResourceType::eStage_output,
		ShaderResourceType::eStorage_buffer,
		ShaderResourceType::ePushConstantBuffer};

	bool failure = false;

	for (const auto& shader : aShaders)
	{
		for (const auto& shaderResource : shader->mReflectionData.mResources)
		{

			if (std::find(exclusions.begin(), exclusions.end(), shaderResource.mType) != exclusions.end())
			{
				continue;
			}

			bool matchFound = false;
			for (auto& shaderResourceFinal : tShaderResources)
			{
				// If set number is the same, and the binding is the same, but the name is different, that would be an issue
				// If set number is the same, the name is the same, but the binding is different, that would be an issue

				bool tSameName = false;
				bool tSameSet = false;
				bool tSameBinding = false;
				bool tSameSize = false;
				bool tSameType = false;

				if (shaderResource.mName == shaderResourceFinal.mName)
				{
					tSameName = true;
				}

				if (shaderResource.mSetNumber == shaderResourceFinal.mSetNumber)
				{
					tSameSet = true;
				}

				if (shaderResource.mBindingNumber == shaderResourceFinal.mBindingNumber)
				{
					tSameBinding = true;
				}

				if (shaderResource.mSize == shaderResourceFinal.mSize)
				{
					tSameSize = true;
				}

				if (shaderResource.mType == shaderResourceFinal.mType)
				{
					tSameType = true;
				}


				// If it is completely equal
				if (tSameName && tSameSet && tSameBinding && tSameSize && tSameType)
				{
					matchFound = true;
					shaderResourceFinal.mShaderAccess |= shaderResource.mShaderAccess; // add access for other shaders
					break;
				}


				else
				{
					bool failureReached = false;

					// If name matches but other data mismatches
					if (tSameName)
					{
						// Has the same name but the other properties are not equal
						if (!tSameSet || !tSameBinding || !tSameSize || !tSameType)
						{
							failureReached = true;
						}

					}
					else
					{
						if (tSameSet && tSameBinding)
						{
							failureReached = true;
						}
					}

					if (failureReached)
						goto break_out;
				}

			}

			if (!matchFound)
			{
				tShaderResources.push_back(shaderResource);
			}
		}
	}

	return tShaderResources;

break_out:
	return std::vector<ShaderResourceReflection>();

}

std::vector<PushConstantReflection> Flux::Gfx::ShaderReflection::MergeRootConstants(const std::vector<std::shared_ptr<Flux::Gfx::Shader>> aShaders)
{
	if (aShaders.size() == 0)
	{
		return std::vector<PushConstantReflection>();
	}

	std::vector<PushConstantReflection> tPushConstants = aShaders[0]->mReflectionData.mPushConstants;

	// Find the shader with the most push constants
	int pushConstantsSize = 0;
	for (const auto& shader : aShaders)
	{
		const auto& pushcnsts = shader->mReflectionData.mPushConstants;

		if (pushcnsts.size() > pushConstantsSize)
		{
			pushConstantsSize = pushcnsts.size();
			tPushConstants = shader->mReflectionData.mPushConstants;
		}
	}


	for (auto& pushConstantBuffer : tPushConstants)
	{
		for (auto& shader : aShaders)
		{
			// FLUX_TODO this code would break if there would be more than one root constant.
			if (shader->mReflectionData.mPushConstants.size() != 0)
			{
				pushConstantBuffer.mShaderAccess |= shader->mShadertype;
			}
		}
	}

	return tPushConstants;
}
