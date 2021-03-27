#include "Renderer.h"

using namespace Flux;

using namespace Flux::Gfx;
using namespace Flux::Gfx::ShaderReflection;


#include <map>

inline VkShaderStageFlags ConvertShaderStageToVkStage(ShaderTypes aShaderAccessFlags)
{

	uint32_t shaderFlags = VkShaderStageFlags(0);

	switch (aShaderAccessFlags)
	{
	if(aShaderAccessFlags & eVertex)
		shaderFlags |= VK_SHADER_STAGE_VERTEX_BIT;
	if(aShaderAccessFlags & eFragment)
		shaderFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if(aShaderAccessFlags & eGeometry)
		shaderFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
	if(aShaderAccessFlags & eTessellationControl)
		shaderFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	if(aShaderAccessFlags & eTessellationEval)
		shaderFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	if(aShaderAccessFlags & eCompute)
		shaderFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
	if(aShaderAccessFlags & eRayGen)
		shaderFlags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	if(aShaderAccessFlags & eRayClosestHit)
		shaderFlags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	if(aShaderAccessFlags & eRayMiss)
		shaderFlags |= VK_SHADER_STAGE_MISS_BIT_KHR;
	if(aShaderAccessFlags & eRayAnyHit)
		shaderFlags |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	if(aShaderAccessFlags & eRayIntersection)
		shaderFlags |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	if(aShaderAccessFlags & eRayCallable)
		shaderFlags |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
	default:
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}

	return VkShaderStageFlags(shaderFlags);
}

inline VkDescriptorType ConvertShaderResourceToDescriptorType(ShaderResourceType aResourceFlags)
{
	switch (aResourceFlags)
	{
	case ShaderResourceType::eUniform_buffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case ShaderResourceType::eStorage_buffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		break;
	case ShaderResourceType:: eSubpass_inputs:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		break;
	case ShaderResourceType:: eStorage_images:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	case ShaderResourceType:: eSampled_images:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	case ShaderResourceType:: eAccelerationStructure:
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		break;
	case ShaderResourceType:: eSeparateImages:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		break;
	case ShaderResourceType:: eSeparateSamplers:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
		break;
	case ShaderResourceType::eAtomic_counters:
	case ShaderResourceType::eUnknownResource:
	case ShaderResourceType::eStage_input:
	case ShaderResourceType::eStage_output:
	case ShaderResourceType::ePushConstantBuffer:
	default:
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		break;
	}
}


VkCommandBuffer Renderer::BeginSingleTimeCommands(VkDevice aDevice, VkCommandPool aCmdPool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = aCmdPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(aDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Renderer::EndSingleTimeCommands(VkDevice aDevice, VkQueue aQueue, VkCommandBuffer aCommandBuffer, VkCommandPool aCmdPool) {
	vkEndCommandBuffer(aCommandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &aCommandBuffer;

	vkQueueSubmit(aQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(aQueue);

	vkFreeCommandBuffers(aDevice, aCmdPool, 1, &aCommandBuffer);
}

void Flux::Gfx::Renderer::TransitionImageLayout(VkDevice aDevice, VkQueue aQueue, VkCommandPool aPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(aDevice, aPool);

	TransitionImageLayout(aDevice, aQueue, aPool, image, format, oldLayout, newLayout, mipLevels, commandBuffer);

	EndSingleTimeCommands(aDevice, aQueue, commandBuffer, aPool);

}

void Flux::Gfx::Renderer::TransitionImageLayout(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, VkCommandBuffer aCommandBuffer)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;


	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_GENERAL)
	{
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		aCommandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

void Flux::Gfx::Renderer::CreateBuffer(VkDevice aDevice, VmaAllocator aAllocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage properties, VkBuffer& buffer, VmaAllocation& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = properties;

	vmaCreateBuffer(aAllocator, &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr);
}

std::shared_ptr<RootSignature> Flux::Gfx::Renderer::CreateRootSignature(std::shared_ptr<RenderContext> aRendererContext, const RootSignatureCreateDesc* const aRootSignatureDesc)
{
	// Check if the shader that is given in the description is a valid shader
	// Combine shader resources (we can do some error checking here)
	// Create descriptor set layouts and descriptor sets from the root signature



	// We don't accept root signatures that have 0 shaders
	if (aRootSignatureDesc->mShaders.size() == 0)
	{
		return nullptr;
	}

	{
		for (const auto& shader : aRootSignatureDesc->mShaders)
		{
			// Shader is invalid if it doesn't have an entry point and is an unknown type
			if (shader->mEntryPoint.length() == 0 || shader->mShadertype == ShaderTypes::eUnknownShaderType)
			{
				return nullptr;
			}
		}
	}

	std::shared_ptr<RootSignature> tRootSignature = std::make_shared<RootSignature>();


	// Merge the shader resources from all the shaders
	tRootSignature->mRootSignatureResources = ShaderReflection::ValidateAndMergeShaderResources(aRootSignatureDesc->mShaders);

	// map to hold the descriptors
	std::map<int32_t, std::vector<Flux::Gfx::ShaderReflection::ShaderResourceReflection>> descriptorMap;

	for (auto& shaderResource : tRootSignature->mRootSignatureResources)
	{
		descriptorMap[shaderResource.mSetNumber].push_back(shaderResource);
	}


	for (auto& set : descriptorMap)
	{
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

		for (auto& binding : set.second)
		{
			VkDescriptorSetLayoutBinding descriptorBinding{};
			descriptorBinding.binding = binding.mBindingNumber;
			descriptorBinding.descriptorType = ConvertShaderResourceToDescriptorType(binding.mType);
			descriptorBinding.descriptorCount = binding.mSize;
			descriptorBinding.stageFlags = ConvertShaderStageToVkStage(Flux::Gfx::ShaderTypes(binding.mShaderAccess));
			descriptorBinding.pImmutableSamplers = nullptr;

			descriptorSetLayoutBindings.push_back(descriptorBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
		layoutInfo.pBindings = descriptorSetLayoutBindings.data();

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

		if (vkCreateDescriptorSetLayout(aRendererContext->mDevice->mDevice, &layoutInfo, nullptr, &descriptorSetLayout))
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		tRootSignature->mDescriptorSetLayouts.push_back(descriptorSetLayout);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.setLayoutCount = tRootSignature->mDescriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = tRootSignature->mDescriptorSetLayouts.data();

	if (vkCreatePipelineLayout(aRendererContext->mDevice->mDevice, &pipelineLayoutInfo, nullptr, &tRootSignature->mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	return tRootSignature;
}

void Flux::Gfx::Renderer::DestroyRootSignature(std::shared_ptr<RenderContext> aRendererContext, std::shared_ptr<RootSignature> aRootSignature)
{
	assert(aRootSignature);
	for (auto& descriptorLayouts : aRootSignature->mDescriptorSetLayouts)
	{
		vkDestroyDescriptorSetLayout(aRendererContext->mDevice->mDevice, descriptorLayouts, nullptr);
	}

	vkDestroyPipelineLayout(aRendererContext->mDevice->mDevice, aRootSignature->mPipelineLayout, nullptr);
}


