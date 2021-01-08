#pragma once

#include <stdexcept>
#include <cassert>
#include <vector>
#include <memory>



#include "vulkan/vulkan.h"
#include <VmaUsage.h>

#include "Context.h"

namespace Flux
{
    class BufferVK;
    class TextureVK;
	class Renderer
	{
    public:
        Renderer(GLFWwindow* aWindow);

		VkShaderModule CreateShaderModule(VkDevice aDevice, const std::vector<char>& code);

		VkCommandBuffer BeginSingleTimeCommands(VkDevice aDevice, VkCommandPool aCmdPool);

		void EndSingleTimeCommands(VkDevice aDevice, VkQueue aQueue, VkCommandBuffer aCommandBuffer, VkCommandPool aCmdPool);

		void TransitionImageLayout(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		void CreateBuffer(VkDevice aDevice, VmaAllocator aAllocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage properties, VkBuffer& buffer, VmaAllocation& bufferMemory);

        // This function quickly copies a buffer on the GPU
        void CopyBuffer(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands(aDevice, aCmdPool);

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            this->EndSingleTimeCommands(aDevice, aQueue, commandBuffer, aCmdPool);
        }

        void CopyBufferToImage(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands(aDevice, aCmdPool);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );

            this->EndSingleTimeCommands(aDevice, aQueue, commandBuffer, aCmdPool);
        }

        std::shared_ptr<BufferVK> CreateAndUploadBuffer(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VmaAllocator aAllocator, VmaMemoryUsage aBufferUsage, VkBufferUsageFlags aBufferFlags, void* aData, size_t aDataSize);


        void CreateImage(VmaAllocator aAllocator, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage properties, VkImage& image, VmaAllocation& imageMemory);

        VkImageView CreateImageView(VkDevice aDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

        std::shared_ptr<TextureVK> CreateAndUploadTexture(
            VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VmaAllocator aAllocator,
            uint32_t aWidth, uint32_t aHeight, uint32_t aImgSize, unsigned char* aImageData,
            VkFormat aFormat);

        std::shared_ptr<Flux::Gfx::Context> mContext;
    private:

	};
};
