#include "Renderer.h"

using namespace Flux;

using namespace Flux::Gfx;
using namespace Flux::Gfx::ShaderReflection;


#include <map>


static VkCullModeFlagBits ConvertCullModeToVkCullBit(Flux::Gfx::CullModes aType)
{
	switch (aType)
	{
	case Flux::Gfx::CullModes::eCullFront:
		return VK_CULL_MODE_FRONT_BIT;
		break;
	case Flux::Gfx::CullModes::eCullBack:
		return VK_CULL_MODE_BACK_BIT;
		break;
	case Flux::Gfx::CullModes::eCullFrontAndBack:
		return VK_CULL_MODE_FRONT_AND_BACK;
		break;

	default:
		VkCullModeFlagBits(0);
	}
}

static VkFrontFace ConvertFrontFaceToVkFaceBit(Flux::Gfx::FrontFace aType)
{
	switch (aType)
	{
	case Flux::Gfx::FrontFace::eCounterClockWise:
		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		break;
	case Flux::Gfx::FrontFace::eClockWise:
		return VK_FRONT_FACE_CLOCKWISE;
		break;

	default:
		VkCullModeFlagBits(0);
	}
}

static VkCompareOp ConvertCompareOpToVkCompareOp(Flux::Gfx::DepthCompareOp aType)
{
	switch (aType)
	{
	case Flux::Gfx::DepthCompareOp::eCompareAlways:
		return VK_COMPARE_OP_ALWAYS;
		break;
	case Flux::Gfx::DepthCompareOp::eCompareEqual:
		return VK_COMPARE_OP_EQUAL;
		break;
	case Flux::Gfx::DepthCompareOp::eCompareGreater:
		return VK_COMPARE_OP_GREATER;
		break;
	case Flux::Gfx::DepthCompareOp::eCompareGreaterOrEqual:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
		break;
	case Flux::Gfx::DepthCompareOp::eCompareLess:
		return VK_COMPARE_OP_LESS;
		break;
	case Flux::Gfx::DepthCompareOp::eCompareLessOrEqual:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
		break;
	case Flux::Gfx::DepthCompareOp::eCompareNever:
		return VK_COMPARE_OP_NEVER;
		break;
	case Flux::Gfx::DepthCompareOp::eCompareNotEqual:
		return VK_COMPARE_OP_NOT_EQUAL;
		break;
	default:
		return VK_COMPARE_OP_MAX_ENUM; // Erorr basically
	}
}

static VkPipelineRasterizationStateCreateInfo CreateRasterInfoFromGenericInfo(Flux::Gfx::RasterizerState state)
{
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = ConvertCullModeToVkCullBit(state.cullMode);
	rasterizer.frontFace = ConvertFrontFaceToVkFaceBit(state.frontFaceMode);
	rasterizer.depthBiasEnable = VK_FALSE;

	return rasterizer;
}

static VkPipelineDepthStencilStateCreateInfo CreateDepthStencilInfoFromGenericInfo(Flux::Gfx::DepthStencilState state)
{
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = state.depthTestEnable;
	depthStencil.depthWriteEnable = state.depthWriteEnable;
	depthStencil.depthCompareOp = ConvertCompareOpToVkCompareOp(state.depthCompareOp);
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	return depthStencil;
}

inline VkShaderStageFlags ConvertShaderStageToVkStage(ShaderTypes aShaderAccessFlags)
{
	uint32_t shaderFlags = VkShaderStageFlags(0);

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

void Flux::Gfx::Renderer::TransitionImageLayout(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, VkCommandBuffer aCommandBuffer, VkImageAspectFlagBits aspectFlag)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspectFlag;
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
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
	barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

	sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	destinationStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	sourceStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
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
			if (shader->mReflectionData.mEntryPoint.length() == 0 || shader->mShadertype == ShaderTypes::eUnknownShaderType)
			{
				return nullptr;
			}
		}
	}

	std::shared_ptr<RootSignature> tRootSignature = std::make_shared<RootSignature>();

	tRootSignature->mShaders.insert(tRootSignature->mShaders.begin(), aRootSignatureDesc->mShaders.begin(), aRootSignatureDesc->mShaders.end());

	// Merge the shader resources from all the shaders
	tRootSignature->mRootSignatureResources = ShaderReflection::ValidateAndMergeShaderResources(aRootSignatureDesc->mShaders);
	tRootSignature->mPushConstantBuffers = ShaderReflection::MergeRootConstants(aRootSignatureDesc->mShaders);


	// map to hold the descriptors
	std::map<int32_t, std::vector<Flux::Gfx::ShaderReflection::ShaderResourceReflection>> descriptorMap;

	for (auto& shaderResource : tRootSignature->mRootSignatureResources)
	{
		descriptorMap[shaderResource.mSetNumber].push_back(shaderResource);
	}

	// Go through every set, turn them into vk descriptor set layout bindings
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


	bool tContainsPushConstants = false;

	VkPushConstantRange pushConstantRange{};
	if (tRootSignature->mPushConstantBuffers.size() != 0)
	{
		const auto& pushCb = tRootSignature->mPushConstantBuffers[0];

		pushConstantRange.stageFlags = ConvertShaderStageToVkStage(Flux::Gfx::ShaderTypes(pushCb.mShaderAccess));
		pushConstantRange.offset = 0;
		pushConstantRange.size = pushCb.mSize;

		tContainsPushConstants = true;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pushConstantRangeCount = tContainsPushConstants ? 1 : 0; // TODO hardcoded for now, figure out if multiple push constants are supported yet.
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
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

std::shared_ptr<Gfx::Shader> Flux::Gfx::Renderer::CreateShader(std::shared_ptr<RenderContext> aContext, const ShaderCreateDesc* const aShaderDesc)
{
	std::shared_ptr<Gfx::Shader> tShader = std::make_shared<Gfx::Shader>();

	// Only reflect after the shader module is guarenteed to have succeeded
	tShader->mReflectionData = ShaderReflection::Reflect(aShaderDesc->mCode);
	tShader->mFilePath = aShaderDesc->mFilePath;
	tShader->mShadertype = aShaderDesc->mType; // Could get the type from reflection, have to consider what is best..
	tShader->mShaderModule = Renderer::CreateShaderModule(aContext->mDevice->mDevice, aShaderDesc->mCode);

	return tShader;
}

void Flux::Gfx::Renderer::DestroyShader(std::shared_ptr<RenderContext> aContext, std::shared_ptr<Gfx::Shader> aShader)
{
	assert(aShader);
	vkDestroyShaderModule(aContext->mDevice->mDevice, aShader->mShaderModule, nullptr);
}

std::shared_ptr<Gfx::GraphicsPipeline> Flux::Gfx::Renderer::CreateGraphicsPipeline(std::shared_ptr<RenderContext> aContext, const GraphicsPipelineCreateDesc* const aPipelineDesc)
{
	assert(aPipelineDesc);

	std::shared_ptr<Gfx::GraphicsPipeline> tGraphicsPipeline = std::make_shared<GraphicsPipeline>();
	tGraphicsPipeline->mRootSignature = aPipelineDesc->mRootSig;

	std::vector<VkPipelineShaderStageCreateInfo> pipelineCreateInfo;

	// Prepare the pipeline stages
	std::shared_ptr<RootSignature> tRootSig = aPipelineDesc->mRootSig.lock();

	for (auto& e : tRootSig->mShaders)
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = ConvertShaderToVkShaderStageBit(e->mShadertype);
		shaderStageInfo.module = e->mShaderModule;
		shaderStageInfo.pName = e->mReflectionData.mEntryPoint.c_str();

		pipelineCreateInfo.push_back(shaderStageInfo);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(aPipelineDesc->vertexAttrDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &aPipelineDesc->bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = aPipelineDesc->vertexAttrDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkExtent2D viewExtent = { aPipelineDesc->mRt.lock()->mWidth, aPipelineDesc->mRt.lock()->mHeight };

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)aPipelineDesc->mRt.lock()->mWidth;
	viewport.height = (float)aPipelineDesc->mRt.lock()->mHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = viewExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;


	VkPipelineRasterizationStateCreateInfo rasterizer = CreateRasterInfoFromGenericInfo(aPipelineDesc->mRasterizer);
	VkPipelineDepthStencilStateCreateInfo depthStencil = CreateDepthStencilInfoFromGenericInfo(aPipelineDesc->mDepthStencilState);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = pipelineCreateInfo.size();
	pipelineInfo.pStages = pipelineCreateInfo.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.layout = tRootSig->mPipelineLayout;
	pipelineInfo.renderPass = aPipelineDesc->mRt.lock()->mPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(aContext->mDevice->mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &tGraphicsPipeline->pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	return tGraphicsPipeline;
}

void Flux::Gfx::Renderer::DestroyGraphicsPipeline(std::shared_ptr<RenderContext> aContext, std::shared_ptr<Gfx::GraphicsPipeline> aGraphicsPipeline)
{
	assert(aGraphicsPipeline);

	vkDestroyPipeline(aContext->mDevice->mDevice, aGraphicsPipeline->pipeline, nullptr);
}

std::shared_ptr<Gfx::ComputePipeline> Flux::Gfx::Renderer::CreateComputePipeline(std::shared_ptr<RenderContext> aContext, const ComputePipelineCreatedesc* const aPipelineDesc)
{
	assert(aPipelineDesc);
	assert(aPipelineDesc->mRootSig.lock());

	std::shared_ptr<Gfx::ComputePipeline> tComputePipeline = std::make_shared<Gfx::ComputePipeline>();
	tComputePipeline->mRootSignature = aPipelineDesc->mRootSig;
	const auto tRootSignature = tComputePipeline->mRootSignature.lock();

	assert(tRootSignature->mShaders.size() == 1);

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageInfo.module = tRootSignature->mShaders[0]->mShaderModule;
	shaderStageInfo.pName = tRootSignature->mShaders[0]->mReflectionData.mEntryPoint.c_str();

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = shaderStageInfo;
	pipelineInfo.layout = tRootSignature->mPipelineLayout;

	vkCreateComputePipelines(aContext->mDevice->mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &tComputePipeline->computePipeline);

	return tComputePipeline;
}

void Flux::Gfx::Renderer::DestroyComputePipeline(std::shared_ptr<RenderContext> aContext, std::shared_ptr<Gfx::ComputePipeline> aComputePipeline)
{
	vkDestroyPipeline(aContext->mDevice->mDevice, aComputePipeline->computePipeline, nullptr);
}