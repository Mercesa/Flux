
#include "CustomRenderer.h"

#include "Application/Camera.h"
#include "Application/Scene/iSceneObject.h"

#include "Renderer/SwapchainVK.h"
#include "Common/AssetProcessing/ModelReaderAssimp.h"


#include <stb_image.h>
using namespace Flux;

#include "Renderer/BufferVK.h"

Flux::CustomRenderer::CustomRenderer(GLFWwindow* aWindow) : mVsync(true)
{
    mRenderer = std::make_shared<Renderer>(aWindow);
    mResourceManager = std::unique_ptr<RenderingResourceManager>(new RenderingResourceManager());
}

void Flux::CustomRenderer::Init()
{
    InitVulkan();
}

void Flux::CustomRenderer::WaitIdle()
{
   vkDeviceWaitIdle(mRenderer->mContext->device);
}

void CustomRenderer::CustomRenderer::InitVulkan() {

    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateCommandPool();
    CreateDescriptorPool();
    CreateDepthResources();
    CreateFramebuffers();
    CreateTextureSampler();
    CreateUniformBuffers();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();


    uint32_t emptyData[1];
    emptyData[0] = 0xFF000000;
    mEmptyTexture =  mRenderer->CreateAndUploadTexture(
        mRenderer->mContext->device, mRenderer->mContext->graphicsQueue, commandPool, mRenderer->mContext->memoryAllocator,
        1, 1,
        sizeof(uint32_t),
        reinterpret_cast<unsigned char*>(emptyData), VK_FORMAT_R8G8B8A8_UNORM);
}

void CustomRenderer::CustomRenderer::MainLoop() {
    vkDeviceWaitIdle(mRenderer->mContext->device);
}

void CustomRenderer::CustomRenderer:: CleanupSwapChain() {

    vmaFreeMemory(this->mRenderer->mContext->memoryAllocator, mRenderer->mContext->mSwapchain->depthImageMemory);
    vkDestroyImage(mRenderer->mContext->device, mRenderer->mContext->mSwapchain->depthImage, nullptr);
    vkDestroyImageView(mRenderer->mContext->device, mRenderer->mContext->mSwapchain->depthImageView, nullptr);

    for (auto framebuffer : mRenderer->mContext->mSwapchain->mFramebuffers) {
        vkDestroyFramebuffer(mRenderer->mContext->device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(mRenderer->mContext->device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());


    for (auto& e : mPipelines)
    {
        vkDestroyPipeline(mRenderer->mContext->device, e.second, nullptr);
    }

    vkDestroyPipelineLayout(mRenderer->mContext->device, pipelineLayoutSceneObjects, nullptr);
    vkDestroyRenderPass(mRenderer->mContext->device, renderPass, nullptr);

    for (auto imageView : mRenderer->mContext->mSwapchain->mImageViews) {
        vkDestroyImageView(mRenderer->mContext->device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(mRenderer->mContext->device, mRenderer->mContext->mSwapchain->mSwapChain, nullptr);

    for (size_t i = 0; i < mRenderer->mContext->mSwapchain->mImages.size(); i++) {
        vkDestroyBuffer(mRenderer->mContext->device, uniformBufferCameraBuffer[i], nullptr);
        vmaFreeMemory(this->mRenderer->mContext->memoryAllocator, uniformBufferCameraMemory[i]);
    }

    vkFreeDescriptorSets(mRenderer->mContext->device, descriptorPool, descriptorSetsSceneObjects.size(), descriptorSetsSceneObjects.data());
}

void CustomRenderer::Cleanup() {
    CleanupSwapChain();

    vkDestroySampler(mRenderer->mContext->device, textureSampler, nullptr);

    vkDestroyImageView(mRenderer->mContext->device, mEmptyTexture->mView, nullptr);
    vkDestroyImage(mRenderer->mContext->device, mEmptyTexture->mImage, nullptr);
    vmaFreeMemory(mRenderer->mContext->memoryAllocator, mEmptyTexture->mAllocation);


    vkDestroyDescriptorPool(mRenderer->mContext->device, this->descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(mRenderer->mContext->device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(mRenderer->mContext->device, descriptorSetLayoutSceneObjects, nullptr);


    for (auto& buffer : mSceneBuffers)
    {
        vkDestroyBuffer(mRenderer->mContext->device, buffer->mBuffer, nullptr);
        vmaFreeMemory(mRenderer->mContext->memoryAllocator, buffer->mAllocation);
    }

    for (auto& texture : mSceneTextures)
    {
        vkDestroyImageView(mRenderer->mContext->device, texture->mView, nullptr);
        vkDestroyImage(mRenderer->mContext->device, texture->mImage, nullptr);
        vmaFreeMemory(mRenderer->mContext->memoryAllocator, texture->mAllocation);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(mRenderer->mContext->device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(mRenderer->mContext->device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(mRenderer->mContext->device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(mRenderer->mContext->device, commandPool, nullptr);

    glfwTerminate();
}

void CustomRenderer::RecreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mRenderer->mContext->device);

    CleanupSwapChain();

    CreateImageViews();
    CreateRenderPass();
    CreateFramebuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
}

VkImageView CustomRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask) {
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
    if (vkCreateImageView(mRenderer->mContext->device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void CustomRenderer::CreateImageViews() {
    mRenderer->mContext->mSwapchain->mImageViews.resize(mRenderer->mContext->mSwapchain->mImages.size());

    for (size_t i = 0; i < mRenderer->mContext->mSwapchain->mImages.size(); i++) {
        mRenderer->mContext->mSwapchain->mImageViews[i] = CreateImageView(mRenderer->mContext->mSwapchain->mImages[i], mRenderer->mContext->mSwapchain->mImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void CustomRenderer::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mRenderer->mContext->mSwapchain->mImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = mRenderer->mContext->FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(mRenderer->mContext->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

VkShaderStageFlagBits ConvertShaderToVkShaderStageBit(ShaderTypes aType)
{
	switch (aType)
	{
	case ShaderTypes::eVertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case ShaderTypes::eFragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
		break;

	default:
		VkShaderStageFlagBits(0);
	}
}

VkCullModeFlagBits ConvertCullModeToVkCullBit(CullModes aType)
{
    switch (aType)
    {
    case CullModes::eCullFront:
        return VK_CULL_MODE_FRONT_BIT;
        break;
    case CullModes::eCullBack:
        return VK_CULL_MODE_BACK_BIT;
        break;
    case CullModes::eCullFrontAndBack:
        return VK_CULL_MODE_FRONT_AND_BACK;
        break;

    default:
        VkCullModeFlagBits(0);
    }
}

VkFrontFace ConvertFrontFaceToVkFaceBit(FrontFace aType)
{
    switch (aType)
    {
    case FrontFace::eCounterClockWise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    case FrontFace::eClockWise:
        return VK_FRONT_FACE_CLOCKWISE;
        break;

    default:
        VkCullModeFlagBits(0);
    }
}

VkPipeline Flux::CustomRenderer::CreateGraphicsPipelineForState(RenderState state)
{
    std::vector<VkShaderModule> modules;
    std::vector<VkPipelineShaderStageCreateInfo> pipelineCreateInfo;

    // Prepare the pipeline stages
    for (auto& e : state.shaders)
    {
        auto shaderCode = readFile(e.second);

        VkShaderModule shaderModule = mRenderer->CreateShaderModule(mRenderer->mContext->device, shaderCode);

        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = ConvertShaderToVkShaderStageBit(e.first);
        shaderStageInfo.module = shaderModule;
        shaderStageInfo.pName = "main";

        modules.push_back(shaderModule);
        pipelineCreateInfo.push_back(shaderStageInfo);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(VertexData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> vertexAttrDescriptions;
    vertexAttrDescriptions.resize(5);

    vertexAttrDescriptions[0].binding = 0;
    vertexAttrDescriptions[0].location = 0;
    vertexAttrDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrDescriptions[0].offset = offsetof(VertexData, position);

    vertexAttrDescriptions[1].binding = 0;
    vertexAttrDescriptions[1].location = 1;
    vertexAttrDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttrDescriptions[1].offset = offsetof(VertexData, texCoords);

    vertexAttrDescriptions[2].binding = 0;
    vertexAttrDescriptions[2].location = 2;
    vertexAttrDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrDescriptions[2].offset = offsetof(VertexData, normal);

    vertexAttrDescriptions[3].binding = 0;
    vertexAttrDescriptions[3].location = 3;
    vertexAttrDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrDescriptions[3].offset = offsetof(VertexData, tangent);

    vertexAttrDescriptions[4].binding = 0;
    vertexAttrDescriptions[4].location = 4;
    vertexAttrDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrDescriptions[4].offset = offsetof(VertexData, bitangent);

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttrDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)mRenderer->mContext->mSwapchain->mExtent.width;
    viewport.height = (float)mRenderer->mContext->mSwapchain->mExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = mRenderer->mContext->mSwapchain->mExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = ConvertCullModeToVkCullBit(state.drawState.cullMode);
    rasterizer.frontFace = ConvertFrontFaceToVkFaceBit(state.drawState.frontFaceMode);
    rasterizer.depthBiasEnable = VK_FALSE;

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

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;


    VkDescriptorSetLayout layouts[2] = { descriptorSetLayout, descriptorSetLayoutSceneObjects };
    pipelineLayoutInfo.pSetLayouts = layouts;

    if (vkCreatePipelineLayout(mRenderer->mContext->device, &pipelineLayoutInfo, nullptr, &pipelineLayoutSceneObjects) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }


    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional


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
    pipelineInfo.layout = pipelineLayoutSceneObjects;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline tPipeline;
    if (vkCreateGraphicsPipelines(mRenderer->mContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &tPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    for (auto& shaderModule : modules)
    {
        vkDestroyShaderModule(mRenderer->mContext->device, shaderModule, nullptr);
    }

    return tPipeline;
}

std::optional<uint32_t> Flux::CustomRenderer::QueryPipeline(RenderState state)
{
    for (uint32_t i = 0; i < mPipelines.size(); ++i)
    {
        if (mPipelines[i].first == state)
        {
            return std::optional<uint32_t>(i);
        }
    }

    return std::optional<uint32_t>();
}

uint32_t Flux::CustomRenderer::CreatePipeline(RenderState state)
{
    auto tQuery = QueryPipeline(state);

    if (tQuery.has_value())
    {
        return tQuery.value();
    }

    this->mPipelines.push_back({state, CreateGraphicsPipelineForState(state) });

    return this->mPipelines.size() - 1;
}

void CustomRenderer::CreateDescriptorSetLayout()
{
    {
        VkDescriptorSetLayoutBinding uboLayoutBindingCamera{};
        uboLayoutBindingCamera.binding = 0;
        uboLayoutBindingCamera.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBindingCamera.descriptorCount = 1;
        uboLayoutBindingCamera.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBindingCamera.pImmutableSamplers = nullptr;


        std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBindingCamera };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(mRenderer->mContext->device, &layoutInfo, nullptr, &descriptorSetLayout))
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    {
        VkDescriptorSetLayoutBinding textureLayoutBinding0{};
        textureLayoutBinding0.binding = 0;
        textureLayoutBinding0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        textureLayoutBinding0.descriptorCount = 1;
        textureLayoutBinding0.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        textureLayoutBinding0.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding textureLayoutBinding1{};
        textureLayoutBinding1.binding = 1;
        textureLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        textureLayoutBinding1.descriptorCount = 1;
        textureLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        textureLayoutBinding1.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding textureLayoutBinding2{};
        textureLayoutBinding2.binding = 2;
        textureLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        textureLayoutBinding2.descriptorCount = 1;
        textureLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        textureLayoutBinding2.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 3;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 4> bindingsSceneObject = { textureLayoutBinding0, textureLayoutBinding1, textureLayoutBinding2, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfoSceneObject{};
        layoutInfoSceneObject.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfoSceneObject.bindingCount = static_cast<uint32_t>(bindingsSceneObject.size());
        layoutInfoSceneObject.pBindings = bindingsSceneObject.data();

        if (vkCreateDescriptorSetLayout(mRenderer->mContext->device, &layoutInfoSceneObject, nullptr, &descriptorSetLayoutSceneObjects))
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

}

void CustomRenderer::CreateFramebuffers() {
    mRenderer->mContext->mSwapchain->mFramebuffers.resize(mRenderer->mContext->mSwapchain->mImageViews.size());

    for (size_t i = 0; i < mRenderer->mContext->mSwapchain->mImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            mRenderer->mContext->mSwapchain->mImageViews[i],
            mRenderer->mContext->mSwapchain->depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = mRenderer->mContext->mSwapchain->mExtent.width;
        framebufferInfo.height = mRenderer->mContext->mSwapchain->mExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mRenderer->mContext->device, &framebufferInfo, nullptr, &mRenderer->mContext->mSwapchain->mFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void CustomRenderer::CreateCommandPool() {
    Flux::Gfx::Context::QueueFamilyIndices queueFamilyIndices = mRenderer->mContext->findQueueFamilies(mRenderer->mContext->physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(mRenderer->mContext->device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CustomRenderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage properties, VkImage& image, VmaAllocation& imageMemory) {
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

    vmaCreateImage(mRenderer->mContext->memoryAllocator, &imageInfo, &allocInfo, &image, &imageMemory, nullptr);
}

void CustomRenderer::CreateDepthResources()
{
    VkFormat depthFormat = mRenderer->mContext->FindDepthFormat();
    CreateImage(mRenderer->mContext->mSwapchain->mExtent.width, mRenderer->mContext->mSwapchain->mExtent.height, depthFormat,
    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
    mRenderer->mContext->mSwapchain->depthImage, mRenderer->mContext->mSwapchain->depthImageMemory);
    mRenderer->mContext->mSwapchain->depthImageView = CreateImageView(mRenderer->mContext->mSwapchain->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void CustomRenderer::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(mRenderer->mContext->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void CustomRenderer::CreateUniformBuffers()
{
    uniformBufferCameraBuffer.resize(mRenderer->mContext->mSwapchain->mImages.size());
    uniformBufferCameraMemory.resize(mRenderer->mContext->mSwapchain->mImages.size());

    for (size_t i = 0; i < mRenderer->mContext->mSwapchain->mImages.size(); i++)
    {
        mRenderer->CreateBuffer(mRenderer->mContext->device, mRenderer->mContext->memoryAllocator, sizeof(UniformBufferCamera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBufferCameraBuffer[i], uniformBufferCameraMemory[i]);
    }
}

void CustomRenderer::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 512 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8192 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8192 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    descriptorPoolCreateInfo.maxSets = 1000;
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(mRenderer->mContext->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error(" Failed to create descriptor pool");
    }
}

void CustomRenderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(mRenderer->mContext->mSwapchain->mImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(mRenderer->mContext->mSwapchain->mImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSetsSceneObjects.resize(mRenderer->mContext->mSwapchain->mImages.size());
    if (vkAllocateDescriptorSets(mRenderer->mContext->device, &allocInfo, descriptorSetsSceneObjects.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allcoate descriptor sets!");
    }

    for (size_t i = 0; i < mRenderer->mContext->mSwapchain->mImages.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfoCamera{};
        bufferInfoCamera.buffer = uniformBufferCameraBuffer[i];
        bufferInfoCamera.offset = 0;
        bufferInfoCamera.range = sizeof(CustomRenderer::UniformBufferCamera);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSetsSceneObjects[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfoCamera;


        vkUpdateDescriptorSets(mRenderer->mContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void CustomRenderer::CreateCommandBuffers() {
    commandBuffers.resize(mRenderer->mContext->mSwapchain->mFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(mRenderer->mContext->device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CustomRenderer::CreateSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(mRenderer->mContext->mSwapchain->mImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(mRenderer->mContext->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mRenderer->mContext->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mRenderer->mContext->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void CustomRenderer::Draw(const std::shared_ptr<iScene> aScene) {

    // Prepare scene resources

    auto& tSceneObjects = aScene->GetSceneObjects();

    for (auto object : tSceneObjects)
    {
        if (object->mAsset != nullptr && object->mMesh == nullptr)
        {
            object->mMesh = std::make_shared<MeshVK>();
            const auto tAsset = object->mAsset;

            object->mMesh->mVertexBuffer = mRenderer->CreateAndUploadBuffer(
                mRenderer->mContext->device, mRenderer->mContext->graphicsQueue, commandPool, mRenderer->mContext->memoryAllocator,
                VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                tAsset->mVertexData.data(), sizeof(tAsset->mVertexData[0]) * tAsset->mVertexData.size());

            object->mMesh->mIndexBuffer = mRenderer->CreateAndUploadBuffer(
                mRenderer->mContext->device, mRenderer->mContext->graphicsQueue, commandPool, mRenderer->mContext->memoryAllocator,
                VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                tAsset->mIndices.data(), sizeof(tAsset->mIndices[0]) * tAsset->mIndices.size());

            mSceneBuffers.push_back(object->mMesh->mVertexBuffer);
            mSceneBuffers.push_back(object->mMesh->mIndexBuffer);
        }

        if (object->mMaterial->mTextureAsset != nullptr && object->mMaterial->mTextureVK == nullptr)
        {
            const auto& tAsset = object->mMaterial->mTextureAsset;

            auto queryResultTexture = mResourceManager->QueryTextureAssetRegistered(tAsset);
            if (!queryResultTexture.has_value())
            {
                object->mMaterial->mTextureVK = mRenderer->CreateAndUploadTexture(
                    mRenderer->mContext->device, mRenderer->mContext->graphicsQueue, commandPool, mRenderer->mContext->memoryAllocator,
                    tAsset->mWidth, tAsset->mHeight,
                    tAsset->mData.size(),
                    tAsset->mData.data(), VK_FORMAT_R8G8B8A8_UNORM);
                mSceneTextures.push_back(object->mMaterial->mTextureVK);
                mResourceManager->RegisterTextureData({ tAsset, object->mMaterial->mTextureVK });
            }
            else
            {
                object->mMaterial->mTextureVK = queryResultTexture.value();
            }

            auto queryResultMaterial = mResourceManager->QueryMaterialAssetRegistered(object->mMaterial);
            if (!queryResultMaterial)
            {
                std::vector<VkDescriptorSetLayout> layouts = { descriptorSetLayoutSceneObjects };
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = layouts.data();

                std::vector<VkDescriptorSet> mSet = { object->mMaterial->mDescriptorSet };
                if (vkAllocateDescriptorSets(mRenderer->mContext->device, &allocInfo, mSet.data()) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate descriptor sets!");
                }


				VkDescriptorImageInfo albedoImage{};
                albedoImage.imageView = object->mMaterial->mTextureVK->mView;
                albedoImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkDescriptorImageInfo emptyImage{};
                emptyImage.imageView = mEmptyTexture->mView;
                emptyImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


                VkDescriptorImageInfo samplerInfo{};
                samplerInfo.sampler = textureSampler;

				std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = mSet[0];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &albedoImage;

                descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[1].dstSet = mSet[0];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo = &albedoImage;

                descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[2].dstSet = mSet[0];
                descriptorWrites[2].dstBinding = 2;
                descriptorWrites[2].dstArrayElement = 0;
                descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                descriptorWrites[2].descriptorCount = 1;
                descriptorWrites[2].pImageInfo = &albedoImage;

				descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[3].dstSet = mSet[0];
				descriptorWrites[3].dstBinding = 3;
				descriptorWrites[3].dstArrayElement = 0;
				descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				descriptorWrites[3].descriptorCount = 1;
                descriptorWrites[3].pImageInfo = &samplerInfo;

				vkUpdateDescriptorSets(mRenderer->mContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                object->mMaterial->mDescriptorSet = mSet[0];

                //TODO DELETE ALLOCATED DESCRIPTOR SETS
                mResourceManager->RegisterMaterial(object->mMaterial);
            }
            else
            {
                object->mMaterial = queryResultMaterial.value();
            }
        }


        if (!object->mRenderState.stateID.has_value())
        {
            object->mRenderState.stateID = std::optional(CreatePipeline(object->mRenderState));
        }

    }

    vkWaitForFences(mRenderer->mContext->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mRenderer->mContext->device, mRenderer->mContext->mSwapchain->mSwapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(mRenderer->mContext->device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    UpdateUniformBuffer(imageIndex, aScene->GetCamera());

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkResetCommandBuffer(commandBuffers[imageIndex], 0);
    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = mRenderer->mContext->mSwapchain->mFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mRenderer->mContext->mSwapchain->mExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (auto& object : aScene->GetSceneObjects())
    {
        // If object contains no render state, can not render.
        if (!object->mRenderState.stateID.has_value())
        {
            continue;
        }
        vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, this->mPipelines[object->mRenderState.stateID.value()].second);
        std::vector<VkDescriptorSet> objectSets = { descriptorSetsSceneObjects[imageIndex], object->mMaterial->mDescriptorSet };
        vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutSceneObjects, 0, objectSets.size(), objectSets.data(), 0, nullptr);

        VkBuffer vertexBuffers[] = { object->mMesh->mVertexBuffer->mBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[imageIndex], object->mMesh->mIndexBuffer->mBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdPushConstants(
            commandBuffers[imageIndex],
            pipelineLayoutSceneObjects,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(glm::mat4),
            &object->transform);

        vkCmdDrawIndexed(commandBuffers[imageIndex], static_cast<uint32_t>(object->mAsset->mIndices.size()), 1, 0, 0, 0);

    }

    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(mRenderer->mContext->device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(mRenderer->mContext->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mRenderer->mContext->mSwapchain->mSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(mRenderer->mContext->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Flux::CustomRenderer::SetWindow(GLFWwindow* aWindow)
{
    mWindow = aWindow;
}

void CustomRenderer::UpdateUniformBuffer(uint32_t currentImage, std::shared_ptr<Camera> aCam) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferCamera ubo{};
    ubo.view = aCam->GetViewMatrix();
    ubo.projection = aCam->GetProjectionMatrix();
    ubo.position = glm::vec4(aCam->Position, 1.0);
    ubo.angle = aCam->mFov;
    ubo.nearPlane = aCam->nearPlane;
    ubo.farPlane = aCam->farPlane;

    void *data;
    vmaMapMemory(mRenderer->mContext->memoryAllocator, uniformBufferCameraMemory[currentImage], &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(mRenderer->mContext->memoryAllocator, uniformBufferCameraMemory[currentImage]);
}



static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
