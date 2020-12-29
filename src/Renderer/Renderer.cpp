#include "Renderer.h"

#include "Renderer/BufferVK.h"
#include "Renderer/TextureVK.h"

using namespace Flux;


VkShaderModule Renderer::CreateShaderModule(VkDevice aDevice, const std::vector<char>& code) {

	assert(code.size() > 0);
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(aDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
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

void Flux::Renderer::TransitionImageLayout(VkDevice aDevice, VkQueue aQueue, VkCommandPool aPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands(aDevice, aPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
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
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	this->EndSingleTimeCommands(aDevice, aQueue, commandBuffer, aPool);

}

void Flux::Renderer::CreateBuffer(VkDevice aDevice, VmaAllocator aAllocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage properties, VkBuffer& buffer, VmaAllocation& bufferMemory)
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

std::shared_ptr<BufferVK> Renderer::CreateAndUploadBuffer(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VmaAllocator aAllocator, VmaMemoryUsage aBufferUsage, VkBufferUsageFlags aBufferFlags, void* aData, size_t aDataSize)
{
	std::shared_ptr<BufferVK> tReturnBuffer = std::make_shared<BufferVK>();

	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	this->CreateBuffer(aDevice, aAllocator, aDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferMemory);

	void* data;
	vmaMapMemory(aAllocator, stagingBufferMemory, &data);
	memcpy(data, aData, aDataSize);
	vmaUnmapMemory(aAllocator, stagingBufferMemory);

	tReturnBuffer->mMemoryUsage = aBufferUsage;
	tReturnBuffer->mUsageFlags = aBufferFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	CreateBuffer(aDevice, aAllocator, aDataSize, tReturnBuffer->mUsageFlags, tReturnBuffer->mMemoryUsage, tReturnBuffer->mBuffer, tReturnBuffer->mAllocation);
	CopyBuffer(aDevice, aQueue, aCmdPool, stagingBuffer, tReturnBuffer->mBuffer, aDataSize);

	vkDestroyBuffer(aDevice, stagingBuffer, nullptr);
	vmaFreeMemory(aAllocator, stagingBufferMemory);

	return std::move(tReturnBuffer);
}

void Renderer::CreateImage(VmaAllocator aAllocator, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage properties, VkImage& image, VmaAllocation& imageMemory) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = properties;

	vmaCreateImage(aAllocator, &imageInfo, &allocInfo, &image, &imageMemory, nullptr);
}

VkImageView Renderer::CreateImageView(VkDevice aDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectMask) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectMask;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(aDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

std::shared_ptr<TextureVK> Renderer::CreateAndUploadTexture(
	VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VmaAllocator aAllocator,
	uint32_t aWidth, uint32_t aHeight, uint32_t aImgSize, unsigned char* aImageData,
	VkFormat aFormat ) {
	
	std::shared_ptr<TextureVK> tReturnTexture = std::make_shared<TextureVK>();

	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	CreateBuffer(aDevice, aAllocator, aImgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferMemory);


	void* data;
	vmaMapMemory(aAllocator, stagingBufferMemory, &data);
	memcpy(data, aImageData, static_cast<size_t>(aImgSize));
	vmaUnmapMemory(aAllocator, stagingBufferMemory);

	CreateImage(aAllocator, aWidth, aHeight, aFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, tReturnTexture->mImage, tReturnTexture->mAllocation);

	TransitionImageLayout(aDevice, aQueue, aCmdPool, tReturnTexture->mImage, aFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(aDevice, aQueue, aCmdPool, stagingBuffer, tReturnTexture->mImage, aWidth, aHeight);
	TransitionImageLayout(aDevice, aQueue, aCmdPool, tReturnTexture->mImage, aFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	tReturnTexture->mView = CreateImageView(aDevice, tReturnTexture->mImage, aFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	vkDestroyBuffer(aDevice, stagingBuffer, nullptr);
	vmaFreeMemory(aAllocator, stagingBufferMemory);

	return std::move(tReturnTexture);
}