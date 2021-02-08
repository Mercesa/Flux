
#include "CustomRenderer.h"

#include "Application/Camera.h"
#include "Application/Scene/iSceneObject.h"

#include "Renderer/Swapchain.h"
#include "Common/AssetProcessing/ModelReaderAssimp.h"


#include <stb_image.h>
using namespace Flux;
using namespace Flux::Gfx;


#include "Renderer/BufferGPU.h"

Flux::CustomRenderer::CustomRenderer(GLFWwindow* aWindow) : mVsync(true), mWindow(aWindow)
{
    mRenderContext = Renderer::CreateRenderContext("Flux", true, mWindow);
    mSwapchain = Renderer::CreateSwapChain(mRenderContext, mWindow );

    Flux::Gfx::QueueCreateDesc QueueDesc{};

    QueueDesc.mType = Flux::Gfx::eQueueType::QUEUE_TYPE_GRAPHICS;
    mQueueGraphics = Renderer::CreateQueue(mRenderContext, &QueueDesc);

    QueueDesc.mType = Flux::Gfx::eQueueType::QUEUE_TYPE_PRESENT;
    mQueuePresent = Renderer::CreateQueue(mRenderContext, &QueueDesc);

    mRenderer = std::make_shared<Renderer>();
    mResourceManager = std::unique_ptr<RenderingResourceManager>(new RenderingResourceManager());

    Flux::Gfx::DescriptorPoolCreateDesc DescriptorPoolCDesc{};
    DescriptorPoolCDesc.maxDescriptorSets = 1024;

    mDescriptorPool = Renderer::CreateDescriptorPool(mRenderContext, &DescriptorPoolCDesc);
}

void Flux::CustomRenderer::Init()
{
    InitVulkan();
}

void Flux::CustomRenderer::WaitIdle()
{
   vkDeviceWaitIdle(mRenderContext->mDevice->mDevice);
}

void CustomRenderer::CustomRenderer::InitVulkan() {

    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateCommandPool();
    CreateFramebuffers();
    CreateTextureSampler();
    CreateUniformBuffers();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();


    uint32_t emptyData[1];
    emptyData[0] = 0xFF000000;
    mEmptyTexture =  mRenderer->CreateAndUploadTexture(mRenderContext,
        mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
        1, 1,
        sizeof(uint32_t), 1,
        reinterpret_cast<unsigned char*>(emptyData), VK_FORMAT_R8G8B8A8_UNORM);
}

void CustomRenderer::CustomRenderer::MainLoop() {
    vkDeviceWaitIdle(mRenderContext->mDevice->mDevice);
}

void CustomRenderer::CustomRenderer::CleanupSwapChain() {

    for (auto framebuffer : mSwapchain->mFramebuffers) {
        vkDestroyFramebuffer(mRenderContext->mDevice->mDevice, framebuffer, nullptr);
    }

    for (auto& e : mPipelines)
    {
        vkDestroyPipeline(mRenderContext->mDevice->mDevice, e.second, nullptr);
    }

    mPipelines.clear();

    vkDestroyPipelineLayout(mRenderContext->mDevice->mDevice, pipelineLayoutSceneObjects, nullptr);
    vkDestroyRenderPass(mRenderContext->mDevice->mDevice, renderPass, nullptr);

    Renderer::DestroySwapchain(mRenderContext, mSwapchain);

    vkFreeDescriptorSets(mRenderContext->mDevice->mDevice, mDescriptorPool->mPool, descriptorSetsSceneObjects.size(), descriptorSetsSceneObjects.data());

}

void CustomRenderer::Cleanup() {
    CleanupSwapChain();

    vkDestroySampler(mRenderContext->mDevice->mDevice, textureSampler, nullptr);

    vkDestroyImageView(mRenderContext->mDevice->mDevice, mEmptyTexture->mView, nullptr);
    vkDestroyImage(mRenderContext->mDevice->mDevice, mEmptyTexture->mImage, nullptr);
    vmaFreeMemory(mRenderContext->memoryAllocator, mEmptyTexture->mAllocation);

    vkDestroyDescriptorSetLayout(mRenderContext->mDevice->mDevice, descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(mRenderContext->mDevice->mDevice, descriptorSetLayoutSceneObjects, nullptr);

    for (auto& buffer : mSceneBuffers)
    {
        vkDestroyBuffer(mRenderContext->mDevice->mDevice, buffer->mBuffer, nullptr);
        vmaFreeMemory(mRenderContext->memoryAllocator, buffer->mAllocation);
    }

    for (auto& buffer : mLightData.mUniformBuffersLights)
    {
        vkDestroyBuffer(mRenderContext->mDevice->mDevice, buffer->mBuffer, nullptr);
        vmaFreeMemory(mRenderContext->memoryAllocator, buffer->mAllocation);
    }

    for (auto& texture : mSceneTextures)
    {
        vkDestroyImageView(mRenderContext->mDevice->mDevice, texture->mView, nullptr);
        vkDestroyImage(mRenderContext->mDevice->mDevice, texture->mImage, nullptr);
        vmaFreeMemory(mRenderContext->memoryAllocator, texture->mAllocation);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(mRenderContext->mDevice->mDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(mRenderContext->mDevice->mDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(mRenderContext->mDevice->mDevice, inFlightFences[i], nullptr);
    }

    for (size_t i = 0; i < mSwapchain->mImages.size(); i++) {
        vkDestroyBuffer(mRenderContext->mDevice->mDevice, mUniformBuffersCamera[i]->mBuffer, nullptr);
        vmaFreeMemory(this->mRenderContext->memoryAllocator, mUniformBuffersCamera[i]->mAllocation);
    }


    vkFreeCommandBuffers(mRenderContext->mDevice->mDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    Renderer::DestroyDescriptorPool(mRenderContext, mDescriptorPool);

    vkDestroyCommandPool(mRenderContext->mDevice->mDevice, commandPool, nullptr);

    glfwTerminate();

    Renderer::DestroyRenderContext(mRenderContext);
}

void CustomRenderer::RecreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mRenderContext->mDevice->mDevice);

    CleanupSwapChain();

    mSwapchain = Renderer::CreateSwapChain(mRenderContext, mWindow);

    CreateRenderPass();
    CreateFramebuffers();

    Flux::Gfx::DescriptorPoolCreateDesc DescriptorPoolCDesc{};
    DescriptorPoolCDesc.maxDescriptorSets = 1024;

    CreateDescriptorSets();
}

void CustomRenderer::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mSwapchain->mImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = mRenderer->FindDepthFormat(mRenderContext);
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

    if (vkCreateRenderPass(mRenderContext->mDevice->mDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
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

        VkShaderModule shaderModule = mRenderer->CreateShaderModule(mRenderContext->mDevice->mDevice, shaderCode);

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
    viewport.width = (float)mSwapchain->mExtent.width;
    viewport.height = (float)mSwapchain->mExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = mSwapchain->mExtent;

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

    if (vkCreatePipelineLayout(mRenderContext->mDevice->mDevice, &pipelineLayoutInfo, nullptr, &pipelineLayoutSceneObjects) != VK_SUCCESS) {
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
    if (vkCreateGraphicsPipelines(mRenderContext->mDevice->mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &tPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    for (auto& shaderModule : modules)
    {
        vkDestroyShaderModule(mRenderContext->mDevice->mDevice, shaderModule, nullptr);
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

        VkDescriptorSetLayoutBinding uboLayoutBindingLight{};
        uboLayoutBindingLight.binding = 1;
        uboLayoutBindingLight.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBindingLight.descriptorCount = 1;
        uboLayoutBindingLight.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBindingLight.pImmutableSamplers = nullptr;


        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBindingCamera, uboLayoutBindingLight };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(mRenderContext->mDevice->mDevice, &layoutInfo, nullptr, &descriptorSetLayout))
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

        if (vkCreateDescriptorSetLayout(mRenderContext->mDevice->mDevice, &layoutInfoSceneObject, nullptr, &descriptorSetLayoutSceneObjects))
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

}

void CustomRenderer::CreateFramebuffers() {
    mSwapchain->mFramebuffers.resize(mSwapchain->mImageViews.size());

    for (size_t i = 0; i < mSwapchain->mImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            mSwapchain->mImageViews[i],
            mSwapchain->depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = mSwapchain->mExtent.width;
        framebufferInfo.height = mSwapchain->mExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mRenderContext->mDevice->mDevice, &framebufferInfo, nullptr, &mSwapchain->mFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void CustomRenderer::CreateCommandPool() {
    Flux::Gfx::QueueFamilyIndices queueFamilyIndices = mRenderer->findQueueFamilies(mRenderContext->mDevice->mPhysicalDevice, mRenderContext->surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(mRenderContext->mDevice->mDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
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
    samplerInfo.maxLod = 6.0f;

    if (vkCreateSampler(mRenderContext->mDevice->mDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void CustomRenderer::CreateUniformBuffers()
{
    mUniformBuffersCamera.resize(mSwapchain->mImages.size());
    mLightData.mUniformBuffersLights.resize(mSwapchain->mImages.size());

    for (size_t i = 0; i < mSwapchain->mImages.size(); i++)
    {
        {
            mUniformBuffersCamera[i] = std::make_shared<BufferGPU>();
            mRenderer->CreateBuffer(mRenderContext->mDevice->mDevice, mRenderContext->memoryAllocator, sizeof(UniformBufferCamera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, mUniformBuffersCamera[i]->mBuffer, mUniformBuffersCamera[i]->mAllocation);
        }

        {
            mLightData.mUniformBuffersLights[i] = std::make_shared<BufferGPU>();
            mRenderer->CreateBuffer(mRenderContext->mDevice->mDevice, mRenderContext->memoryAllocator, sizeof(UniformBufferLights) * AMOUNT_OF_SUPPORTED_LIGHTS, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, mLightData.mUniformBuffersLights[i]->mBuffer, mLightData.mUniformBuffersLights[i]->mAllocation);
        }
    }
}

void CustomRenderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(mSwapchain->mImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool->mPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapchain->mImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSetsSceneObjects.resize(mSwapchain->mImages.size());
    if (vkAllocateDescriptorSets(mRenderContext->mDevice->mDevice, &allocInfo, descriptorSetsSceneObjects.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allcoate descriptor sets!");
    }

    for (size_t i = 0; i < mSwapchain->mImages.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfoCamera{};
        bufferInfoCamera.buffer = mUniformBuffersCamera[i]->mBuffer;
        bufferInfoCamera.offset = 0;
        bufferInfoCamera.range = sizeof(CustomRenderer::UniformBufferCamera);


        VkDescriptorBufferInfo bufferInfoLight;


		bufferInfoLight.buffer = mLightData.mUniformBuffersLights[i]->mBuffer;
		bufferInfoLight.offset = 0;
		bufferInfoLight.range = sizeof(Light) * AMOUNT_OF_SUPPORTED_LIGHTS;


        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSetsSceneObjects[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfoCamera;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSetsSceneObjects[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &bufferInfoLight;



        vkUpdateDescriptorSets(mRenderContext->mDevice->mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void CustomRenderer::CreateCommandBuffers() {
    commandBuffers.resize(mSwapchain->mFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(mRenderContext->mDevice->mDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CustomRenderer::CreateSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(mSwapchain->mImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(mRenderContext->mDevice->mDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mRenderContext->mDevice->mDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mRenderContext->mDevice->mDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
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
                mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
                VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                tAsset->mVertexData.data(), sizeof(tAsset->mVertexData[0]) * tAsset->mVertexData.size());

            object->mMesh->mIndexBuffer = mRenderer->CreateAndUploadBuffer(
                mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
                VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                tAsset->mIndices.data(), sizeof(tAsset->mIndices[0]) * tAsset->mIndices.size());

            mSceneBuffers.push_back(object->mMesh->mVertexBuffer);
            mSceneBuffers.push_back(object->mMesh->mIndexBuffer);
        }

        if (object->mMaterial->mTextureAssetAlbedo != nullptr && object->mMaterial->mTextureAlbedo == nullptr)
        {
            {
                const auto& tAssetAlbedo = object->mMaterial->mTextureAssetAlbedo;

                auto queryResultTextureAlbedo = mResourceManager->QueryTextureAssetRegistered(tAssetAlbedo);
                if (!queryResultTextureAlbedo.has_value())
                {
                    object->mMaterial->mTextureAlbedo = mRenderer->CreateAndUploadTexture(mRenderContext,
                        mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
                        tAssetAlbedo->mWidth, tAssetAlbedo->mHeight,
                        tAssetAlbedo->mData.size(), tAssetAlbedo->mMipLevels,
                        tAssetAlbedo->mData.data(), VK_FORMAT_R8G8B8A8_SRGB);
                    mSceneTextures.push_back(object->mMaterial->mTextureAlbedo);
                    mResourceManager->RegisterTextureData({ tAssetAlbedo, object->mMaterial->mTextureAlbedo });
                }
                else
                {
                    object->mMaterial->mTextureAlbedo = queryResultTextureAlbedo.value();
                }
            }


            {
                const auto& tAssetNormal = object->mMaterial->mTextureAssetNormal;

                auto queryResultTextureNormal = mResourceManager->QueryTextureAssetRegistered(tAssetNormal);
                if (!queryResultTextureNormal.has_value())
                {
                    object->mMaterial->mTextureNormal = mRenderer->CreateAndUploadTexture(mRenderContext,
                        mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
                        tAssetNormal->mWidth, tAssetNormal->mHeight,
                        tAssetNormal->mData.size(), tAssetNormal->mMipLevels,
                        tAssetNormal->mData.data(), VK_FORMAT_R8G8B8A8_UNORM);
                    mSceneTextures.push_back(object->mMaterial->mTextureNormal);
                    mResourceManager->RegisterTextureData({ tAssetNormal, object->mMaterial->mTextureNormal });
                }
                else
                {
                    object->mMaterial->mTextureNormal = queryResultTextureNormal.value();
                }
            }

            {
                const auto& tAssetSpecular = object->mMaterial->mTextureAssetSpecular;

                auto queryResultTextureSpec = mResourceManager->QueryTextureAssetRegistered(tAssetSpecular);
                if (!queryResultTextureSpec.has_value())
                {
                    object->mMaterial->mTextureSpecular = mRenderer->CreateAndUploadTexture(mRenderContext,
                        mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
                        tAssetSpecular->mWidth, tAssetSpecular->mHeight,
                        tAssetSpecular->mData.size(), tAssetSpecular->mMipLevels,
                        tAssetSpecular->mData.data(), VK_FORMAT_R8G8B8A8_UNORM);
                    mSceneTextures.push_back(object->mMaterial->mTextureSpecular);
                    mResourceManager->RegisterTextureData({ tAssetSpecular, object->mMaterial->mTextureSpecular });
                }
                else
                {
                    object->mMaterial->mTextureSpecular = queryResultTextureSpec.value();
                }
            }


            auto queryResultMaterial = mResourceManager->QueryMaterialAssetRegistered(object->mMaterial);
            if (!queryResultMaterial)
            {
                std::vector<VkDescriptorSetLayout> layouts = { descriptorSetLayoutSceneObjects };
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = mDescriptorPool->mPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = layouts.data();

                std::vector<VkDescriptorSet> mSet = { object->mMaterial->mDescriptorSet };
                if (vkAllocateDescriptorSets(mRenderContext->mDevice->mDevice, &allocInfo, mSet.data()) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate descriptor sets!");
                }


				VkDescriptorImageInfo albedoImage{};
                albedoImage.imageView = object->mMaterial->mTextureAlbedo->mView;
                albedoImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkDescriptorImageInfo specularImage{};
                specularImage.imageView = object->mMaterial->mTextureSpecular->mView;
                specularImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkDescriptorImageInfo normalImage{};
                normalImage.imageView = object->mMaterial->mTextureNormal->mView;
                normalImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
                descriptorWrites[1].pImageInfo = &specularImage;

                descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[2].dstSet = mSet[0];
                descriptorWrites[2].dstBinding = 2;
                descriptorWrites[2].dstArrayElement = 0;
                descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                descriptorWrites[2].descriptorCount = 1;
                descriptorWrites[2].pImageInfo = &normalImage;

				descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[3].dstSet = mSet[0];
				descriptorWrites[3].dstBinding = 3;
				descriptorWrites[3].dstArrayElement = 0;
				descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				descriptorWrites[3].descriptorCount = 1;
                descriptorWrites[3].pImageInfo = &samplerInfo;

				vkUpdateDescriptorSets(mRenderContext->mDevice->mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

    vkWaitForFences(mRenderContext->mDevice->mDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mRenderContext->mDevice->mDevice, mSwapchain->mSwapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(mRenderContext->mDevice->mDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    UpdateUniformBuffer(imageIndex, aScene->GetCamera(), aScene->GetLights());

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkResetCommandBuffer(commandBuffers[imageIndex], 0);
    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = mSwapchain->mFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapchain->mExtent;

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

        if (!QueryPipeline(object->mRenderState))
        {
            object->mRenderState.stateID = std::nullopt;
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

    vkResetFences(mRenderContext->mDevice->mDevice, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(mQueueGraphics->mVkQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mSwapchain->mSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(mQueuePresent->mVkQueue, &presentInfo);

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

void CustomRenderer::UpdateUniformBuffer(uint32_t currentImage, std::shared_ptr<Camera> aCam, std::vector<std::shared_ptr<Light>> aLights) {
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
    vmaMapMemory(mRenderContext->memoryAllocator, mUniformBuffersCamera[currentImage]->mAllocation, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(mRenderContext->memoryAllocator, mUniformBuffersCamera[currentImage]->mAllocation);



    // We need to do this to make sure if any lights are deleted we clean the array, though better to have a dirty flag or something like that
    for (uint32_t lIndex = 0; lIndex < AMOUNT_OF_SUPPORTED_LIGHTS; ++lIndex)
    {
        mLightData.lightCache[lIndex] = Light();
    }

    if (aLights.size() >= AMOUNT_OF_SUPPORTED_LIGHTS)
    {
        std::cout << "Max amount of lights exceeded, won't update light array";
        return;
    }

    for (uint32_t lIndex = 0; lIndex < aLights.size(); ++lIndex)
    {
        mLightData.lightCache[lIndex] = (*aLights[lIndex]);
    }
    mLightData.lightCache[0].amountOfLights = aLights.size();


    void* lightData;
    vmaMapMemory(mRenderContext->memoryAllocator, mLightData.mUniformBuffersLights[currentImage]->mAllocation, &lightData);
    memcpy(lightData, mLightData.lightCache, sizeof(Light) * AMOUNT_OF_SUPPORTED_LIGHTS);
    vmaUnmapMemory(mRenderContext->memoryAllocator, mLightData.mUniformBuffersLights[currentImage]->mAllocation);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
