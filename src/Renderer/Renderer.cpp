#include "Renderer.h"

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

void Renderer::EndSingleTimeCommands(VkDevice aDevice, VkQueue aQueue, VkCommandBuffer aCommandBuffer, VkCommandPool aCmdPool ) {
    vkEndCommandBuffer(aCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &aCommandBuffer;

    vkQueueSubmit(aQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(aQueue);

    vkFreeCommandBuffers(aDevice, aCmdPool, 1, &aCommandBuffer);
}