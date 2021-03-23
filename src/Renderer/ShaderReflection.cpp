#include "ShaderReflection.h"

#include "spirv_reflect.hpp"

using namespace Flux::Gfx::ShaderReflection;

std::vector<ShaderResourceReflection> AddResourcesToList(const spirv_cross::CompilerGLSL& aCompiler, const spirv_cross::SmallVector<spirv_cross::Resource>& aResources, ShaderResourceType aType)
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

		tResourceList.push_back(tResource);
	}

	return tResourceList;
}

ShaderReflection Flux::Gfx::ShaderReflection::Reflect(std::vector<char> aSpvbinary)
{
	ShaderReflection tReflection{};

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

	// Uniform buffers
	{
		auto tUniformBuffers = AddResourcesToList(glsl, resources.uniform_buffers, ShaderResourceType::eUniform_buffer);
		tReflection.mResources.insert(tReflection.mResources.end(), tUniformBuffers.begin(), tUniformBuffers.end());
	}

	// Storage buffers
	{
		auto tStorageBuffer = AddResourcesToList(glsl, resources.uniform_buffers, ShaderResourceType::eStorage_buffer);
		tReflection.mResources.insert(tReflection.mResources.end(), tStorageBuffer.begin(), tStorageBuffer.end());
	}

	// Stage inputs
	{
		auto tStageInputs = AddResourcesToList(glsl, resources.stage_inputs, ShaderResourceType::eStage_input);
		tReflection.mResources.insert(tReflection.mResources.end(), tStageInputs.begin(), tStageInputs.end());
	}

	// Stage outputs
	{
		auto tStageOutputs = AddResourcesToList(glsl, resources.stage_outputs, ShaderResourceType::eStage_output);
		tReflection.mResources.insert(tReflection.mResources.end(), tStageOutputs.begin(), tStageOutputs.end());
	}

	// Subpass inputs
	{
		auto tSubpassInputs = AddResourcesToList(glsl, resources.subpass_inputs, ShaderResourceType::eSubpass_inputs);
		tReflection.mResources.insert(tReflection.mResources.end(), tSubpassInputs.begin(), tSubpassInputs.end());
	}

	// Storage images
	{
		auto tStorageImages = AddResourcesToList(glsl, resources.storage_images, ShaderResourceType::eStorage_images);
		tReflection.mResources.insert(tReflection.mResources.end(), tStorageImages.begin(), tStorageImages.end());
	}

	// Sampled images
	{
		auto tSampledImages = AddResourcesToList(glsl, resources.sampled_images, ShaderResourceType::eSampled_images);
		tReflection.mResources.insert(tReflection.mResources.end(), tSampledImages.begin(), tSampledImages.end());
	}

	// Atomic counters
	{
		auto tAtomicCounters = AddResourcesToList(glsl, resources.atomic_counters, ShaderResourceType::eAtomic_counters);
		tReflection.mResources.insert(tReflection.mResources.end(), tAtomicCounters.begin(), tAtomicCounters.end());
	}

	// Push constants
	{
		auto tPushConstants = AddResourcesToList(glsl, resources.push_constant_buffers, ShaderResourceType::ePushConstantBuffer);
		tReflection.mResources.insert(tReflection.mResources.end(), tPushConstants.begin(), tPushConstants.end());
	}

	// Acceleration structures
	{
		auto tAccelerationStructures = AddResourcesToList(glsl, resources.acceleration_structures, ShaderResourceType::eAccelerationStructure);
		tReflection.mResources.insert(tReflection.mResources.end(), tAccelerationStructures.begin(), tAccelerationStructures.end());
	}

	// Separate image
	{
		auto tSeparateImages = AddResourcesToList(glsl, resources.separate_images, ShaderResourceType::eSeparateImages);
		tReflection.mResources.insert(tReflection.mResources.end(), tSeparateImages.begin(), tSeparateImages.end());
	}

	// Separate sampler
	{
		auto tSeparateSamplers = AddResourcesToList(glsl, resources.separate_samplers, ShaderResourceType::eSeparateSamplers);
		tReflection.mResources.insert(tReflection.mResources.end(), tSeparateSamplers.begin(), tSeparateSamplers.end());
	}

	return tReflection;
}