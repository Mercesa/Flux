
#include "CustomRenderer.h"

#include "Application/Camera.h"
#include "Application/Scene/iSceneObject.h"

#include "Renderer/Swapchain.h"
#include "Common/AssetProcessing/ModelReaderAssimp.h"

#include "Common/FileHandling/FileReadUtility.h"

#include "glm/gtc/type_ptr.hpp"

#include "ImguiRenderingHelper.h"

using namespace Flux;
using namespace Flux::Gfx;


#include "Renderer/BufferGPU.h"
#include "Application/Rendering/Pipelines.h"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

Flux::CustomRenderer::CustomRenderer(GLFWwindow* aWindow) : mVsync(true), mWindow(aWindow)
{
    mRenderContext = Renderer::CreateRenderContext("Flux", true, mWindow);
    mSwapchain = Renderer::CreateSwapChain(mRenderContext, mWindow );

    Flux::Gfx::QueueCreateDesc QueueDesc{};

    QueueDesc.mType = Flux::Gfx::eQueueType::QUEUE_TYPE_GRAPHICS;
    mQueueGraphics = Renderer::CreateQueue(mRenderContext, &QueueDesc);

    QueueDesc.mType = Flux::Gfx::eQueueType::QUEUE_TYPE_PRESENT;
    mQueuePresent = Renderer::CreateQueue(mRenderContext, &QueueDesc);

    mResourceManager = std::unique_ptr<RenderingResourceManager>(new RenderingResourceManager());

    Flux::Gfx::DescriptorPoolCreateDesc DescriptorPoolCDesc{};
    DescriptorPoolCDesc.maxDescriptorSets = 1024;

    mDescriptorPool = Renderer::CreateDescriptorPool(mRenderContext, &DescriptorPoolCDesc);
}

void Flux::CustomRenderer::Init()
{
    InitVulkan();

    {
        ImGui_ImplVulkan_InitInfo imguiInitInfo{};

        imguiInitInfo.Instance = mRenderContext->instance;
        imguiInitInfo.PhysicalDevice = mRenderContext->mDevice->mPhysicalDevice;
        imguiInitInfo.Device = mRenderContext->mDevice->mDevice;
        imguiInitInfo.Queue = mQueueGraphics->mVkQueue;
        imguiInitInfo.QueueFamily = mQueueGraphics->mQueueIndex;
        imguiInitInfo.DescriptorPool = mDescriptorPool->mPool;
        imguiInitInfo.MinImageCount = 2;
        imguiInitInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
        imguiInitInfo.CheckVkResultFn = check_vk_result;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(mWindow, true);
        ImGui_ImplVulkan_Init(&imguiInitInfo, mSwapchain->mRenderPass);

        // Upload fonts texture
        VkCommandBuffer cmdBuffer = Renderer::BeginSingleTimeCommands(mRenderContext->mDevice->mDevice, commandPool);
        ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
        Renderer::EndSingleTimeCommands(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, cmdBuffer, commandPool);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void Flux::CustomRenderer::WaitIdle()
{
   vkDeviceWaitIdle(mRenderContext->mDevice->mDevice);
}

void CustomRenderer::CustomRenderer::InitVulkan() {

    CreateCommandPool();
    CreateTextureSampler();
    CreateUniformBuffers();

    std::shared_ptr<Gfx::Shader> tFragShader = nullptr;
    {
        auto filepath = "Resources/Shaders/basicModel.frag.spv";
        auto codeFrag = Flux::Common::ReadFile<char>(filepath);

        ShaderCreateDesc fragShaderCD{};
        fragShaderCD.mCode = codeFrag;
        fragShaderCD.mFilePath = filepath;
        fragShaderCD.mType = ShaderTypes::eFragment;
        tFragShader = Renderer::CreateShader(mRenderContext, &fragShaderCD);
        mShadersAll.push_back(tFragShader);
    }

    std::shared_ptr<Gfx::Shader> tVertShader = nullptr;
    {
        auto filepath = "Resources/Shaders/basicModel.vert.spv";
        auto codeVert = Flux::Common::ReadFile<char>(filepath);

        ShaderCreateDesc vertShaderCD{};
        vertShaderCD.mCode = codeVert;
        vertShaderCD.mFilePath = filepath;
        vertShaderCD.mType = ShaderTypes::eVertex;
        tVertShader = Renderer::CreateShader(mRenderContext, &vertShaderCD);
        mShadersAll.push_back(tVertShader);
    }

    Gfx::RootSignatureCreateDesc sceneRootSigDesc{};
    sceneRootSigDesc.mShaders = { tFragShader, tVertShader };
    mRootSignatureScene = Renderer::CreateRootSignature(mRenderContext, &sceneRootSigDesc);

    mRootSignaturesAll.push_back(mRootSignatureScene);

    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();

    {
        Flux::Gfx::RenderTargetCreateDesc RTCreateDescScene{};
        RTCreateDescScene.mWidth = mSwapchain->mExtent.width;
        RTCreateDescScene.mHeight = mSwapchain->mExtent.height;
        RTCreateDescScene.mTargets = { {VK_FORMAT_R16G16B16A16_UNORM} };
        RTCreateDescScene.mDepthTarget = { VK_FORMAT_D32_SFLOAT };
        mRenderTargetScene = Renderer::CreateRenderTarget(mRenderContext, mRenderContext->mDevice, mQueueGraphics, commandPool, mRenderContext->memoryAllocator, &RTCreateDescScene);
    }

    uint32_t emptyData[1];
    emptyData[0] = 0xFF000000;
    mEmptyTexture =  Renderer::CreateAndUploadTexture(mRenderContext,
        mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
        1, 1,
        sizeof(uint32_t), 1,
        reinterpret_cast<unsigned char*>(emptyData), VK_FORMAT_R8G8B8A8_UNORM);


    // Ofscreen renderpass
	{
		Flux::Gfx::RenderTargetCreateDesc RTCreateDesc{};
		RTCreateDesc.mWidth = mSwapchain->mExtent.width;
		RTCreateDesc.mHeight = mSwapchain->mExtent.height;
		RTCreateDesc.mTargets = { {VK_FORMAT_R8G8B8A8_UNORM} };
		RTCreateDesc.mDepthTarget = { VK_FORMAT_D32_SFLOAT };
		mRenderTargetFinal = Renderer::CreateRenderTarget(mRenderContext, mRenderContext->mDevice, mQueueGraphics, commandPool, mRenderContext->memoryAllocator, &RTCreateDesc);
	}



    auto codeCompute = Flux::Common::ReadFile<char>("Resources/Shaders/postfx.comp.spv");

    ShaderCreateDesc compShaderCD{};
    compShaderCD.mCode = codeCompute;
    compShaderCD.mFilePath = "Resources/Shaders/postfx.comp.spv";
    compShaderCD.mType = ShaderTypes::eCompute;
    mComputeShader = Renderer::CreateShader(mRenderContext, &compShaderCD);

    RootSignatureCreateDesc rootSigDesc{};
    rootSigDesc.mShaders.push_back(mComputeShader);

    mShadersAll.push_back(mComputeShader);

    mRootSignatureCompute = Renderer::CreateRootSignature(mRenderContext, &rootSigDesc);

    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool->mPool;
        allocInfo.descriptorSetCount = 1u;
        allocInfo.pSetLayouts = mRootSignatureCompute->mDescriptorSetLayouts.data();

        if (vkAllocateDescriptorSets(mRenderContext->mDevice->mDevice, &allocInfo, &mComputeDataPostfx.descriptorset) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        VkDescriptorImageInfo imageInfoComputeRead{};
        imageInfoComputeRead.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
        imageInfoComputeRead.imageView = mRenderTargetScene->mColorImages[0]->mView;

        VkDescriptorImageInfo imageInfoComputeWrite{};
        imageInfoComputeWrite.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
        imageInfoComputeWrite.imageView = mRenderTargetFinal->mColorImages[0]->mView;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mComputeDataPostfx.descriptorset;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfoComputeRead;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = mComputeDataPostfx.descriptorset;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfoComputeWrite;

        vkUpdateDescriptorSets(mRenderContext->mDevice->mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

        ComputePipelineCreatedesc computePipelineCreateDesc{};
        computePipelineCreateDesc.mRootSig = mRootSignatureCompute;

        mComputePipeline = Renderer::CreateComputePipeline(mRenderContext, &computePipelineCreateDesc);
    }



}

void CustomRenderer::CustomRenderer::MainLoop() {
    vkDeviceWaitIdle(mRenderContext->mDevice->mDevice);
}

void CustomRenderer::CustomRenderer::CleanupSwapChain() {
    for (auto& e : mPipelines)
    {
        Renderer::DestroyGraphicsPipeline(mRenderContext, e.second);
    }

    mPipelines.clear();

    Renderer::DestroySwapchain(mRenderContext, mSwapchain);

    vkFreeDescriptorSets(mRenderContext->mDevice->mDevice, mDescriptorPool->mPool, descriptorSetsSceneObjects.size(), descriptorSetsSceneObjects.data());
    Renderer::DestroyRenderTarget(mRenderContext, mRenderTargetScene);
    Renderer::DestroyRenderTarget(mRenderContext, mRenderTargetFinal);
}

void CustomRenderer::Cleanup() {
    CleanupSwapChain();

    // Imgui cleanup
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }


    vkDestroySampler(mRenderContext->mDevice->mDevice, textureSampler, nullptr);

    vkDestroyImageView(mRenderContext->mDevice->mDevice, mEmptyTexture->mView, nullptr);
    vkDestroyImage(mRenderContext->mDevice->mDevice, mEmptyTexture->mImage, nullptr);
    vmaFreeMemory(mRenderContext->memoryAllocator, mEmptyTexture->mAllocation);


    for (auto rootSig : mRootSignaturesAll)
    {
        Renderer::DestroyRootSignature(mRenderContext, rootSig);
    }
    mRootSignaturesAll.clear();

    for (auto pipeline : mPipelines)
    {
        Renderer::DestroyGraphicsPipeline(mRenderContext, pipeline.second);
    }
    mPipelines.clear();

    for (auto shader : mShadersAll)
    {
        Renderer::DestroyShader(mRenderContext, shader);
    }

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


    Renderer::DestroyComputePipeline(mRenderContext, mComputePipeline);
	Renderer::DestroyRootSignature(mRenderContext, mRootSignatureCompute);

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

    {
        Flux::Gfx::RenderTargetCreateDesc RTCreateDesc{};
        RTCreateDesc.mWidth = mSwapchain->mExtent.width;
        RTCreateDesc.mHeight = mSwapchain->mExtent.height;
        RTCreateDesc.mTargets = { {VK_FORMAT_R16G16B16A16_UNORM} };
        RTCreateDesc.mDepthTarget = { VK_FORMAT_D32_SFLOAT };
        mRenderTargetScene = Renderer::CreateRenderTarget(mRenderContext, mRenderContext->mDevice, mQueueGraphics, commandPool, mRenderContext->memoryAllocator, &RTCreateDesc);
    }

    {
        Flux::Gfx::RenderTargetCreateDesc RTCreateDesc{};
        RTCreateDesc.mWidth = mSwapchain->mExtent.width;
        RTCreateDesc.mHeight = mSwapchain->mExtent.height;
        RTCreateDesc.mTargets = { {VK_FORMAT_R8G8B8A8_UNORM} };
        RTCreateDesc.mDepthTarget = { VK_FORMAT_D32_SFLOAT };
        mRenderTargetFinal = Renderer::CreateRenderTarget(mRenderContext, mRenderContext->mDevice, mQueueGraphics, commandPool, mRenderContext->memoryAllocator, &RTCreateDesc);
    }

    CreateDescriptorSets();
}

std::optional<std::shared_ptr<Flux::Gfx::Shader>> CustomRenderer::DoesShaderExist(std::string aFilePath)
{
    for (auto& shader : mShadersAll)
    {
        if (shader->mFilePath == aFilePath)
        {
            return std::optional<std::shared_ptr<Flux::Gfx::Shader>>(shader);
        }
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Flux::Gfx::RootSignature>> Flux::CustomRenderer::DoesRootSignatureExist(const std::vector<std::shared_ptr<Flux::Gfx::Shader>>& aShaders)
{
    // Go through all root signatures, check if there is a matching shader list to an existing root signature
    for (const auto& rootSig : mRootSignaturesAll)
    {
        if (rootSig->mShaders.size() != aShaders.size())
        {
            continue;
        }

        bool match = true;

        for (const auto& shader : aShaders)
        {
            bool shaderMatch = false;

            // Check if we can find a matching shader from the root signature
            // if not then there is no match and this root sig doesn't match our input
            for (const auto& shaderRootSig : rootSig->mShaders)
            {
                if (shaderRootSig->mFilePath == shader->mFilePath)
                {
                    shaderMatch = true;
                    break;
                }
            }

            if (!shaderMatch)
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            return std::optional<std::shared_ptr<Flux::Gfx::RootSignature>>(rootSig);
        }
    }

    return std::optional<std::shared_ptr<Flux::Gfx::RootSignature>>();
}

std::shared_ptr<Flux::Gfx::GraphicsPipeline> Flux::CustomRenderer::CreateGraphicsPipelineForState(RenderState state)
{

    std::vector<std::shared_ptr<Flux::Gfx::Shader>> tShaders;

    // Prepare the pipeline stages
    for (auto& e : state.shaders)
    {
        std::shared_ptr<Flux::Gfx::Shader> tShader;

        auto shaderOptional = DoesShaderExist(e.second);

        if (!shaderOptional.has_value())
        {
            auto shaderCode = Common::ReadFile<char>(e.second);

            ShaderCreateDesc shaderCreateDesc{};
            shaderCreateDesc.mCode = shaderCode;
            shaderCreateDesc.mFilePath = e.second;
            shaderCreateDesc.mType = e.first;
            tShader = Renderer::CreateShader(mRenderContext, &shaderCreateDesc);
            mShadersAll.push_back(tShader);
        }
        else
        {
            tShader = shaderOptional.value();
        }

        tShaders.push_back(tShader);
    }

    auto resultRootQuery = DoesRootSignatureExist(tShaders);

    std::shared_ptr<RootSignature> tRootSig = nullptr;
    if (resultRootQuery.has_value())
    {
        tRootSig = resultRootQuery.value();
    }
    else
    {
        RootSignatureCreateDesc rootSigCreateDesc{};
        rootSigCreateDesc.mShaders = tShaders;

        tRootSig = Renderer::CreateRootSignature(mRenderContext, &rootSigCreateDesc);

        mRootSignaturesAll.push_back(tRootSig);
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

    Flux::Gfx::GraphicsPipelineCreateDesc pipelineDesc;
    pipelineDesc.vertexAttrDescriptions = vertexAttrDescriptions;
    pipelineDesc.bindingDescription = bindingDescription;
    pipelineDesc.mPushConstantSize = sizeof(glm::mat4);
    pipelineDesc.mRootSig = tRootSig;
    pipelineDesc.mRt = mRenderTargetScene;



    auto tPipeline = Renderer::CreateGraphicsPipeline(mRenderContext, &pipelineDesc);

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

void CustomRenderer::CreateCommandPool() {
    Flux::Gfx::QueueFamilyIndices queueFamilyIndices = Renderer::findQueueFamilies(mRenderContext->mDevice->mPhysicalDevice, mRenderContext->surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value().first;
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
            Renderer::CreateBuffer(mRenderContext->mDevice->mDevice, mRenderContext->memoryAllocator, sizeof(UniformBufferCamera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, mUniformBuffersCamera[i]->mBuffer, mUniformBuffersCamera[i]->mAllocation);
        }

        {
            mLightData.mUniformBuffersLights[i] = std::make_shared<BufferGPU>();
            Renderer::CreateBuffer(mRenderContext->mDevice->mDevice, mRenderContext->memoryAllocator, sizeof(UniformBufferLights) * AMOUNT_OF_SUPPORTED_LIGHTS, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, mLightData.mUniformBuffersLights[i]->mBuffer, mLightData.mUniformBuffersLights[i]->mAllocation);
        }
    }
}

void CustomRenderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(mSwapchain->mImages.size(), mRootSignatureScene->mDescriptorSetLayouts[0]);
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
    commandBuffers.resize(mSwapchain->mImages.size());

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

    // Imgui start

    {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // Prepare scene resources
    auto& tSceneObjects = aScene->GetSceneObjects();

    for (auto object : tSceneObjects)
    {
        if (object->mAsset != nullptr && object->mMesh == nullptr)
        {
            object->mMesh = std::make_shared<MeshVK>();
            const auto tAsset = object->mAsset;

            object->mMesh->mVertexBuffer = Renderer::CreateAndUploadBuffer(
                mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderContext->memoryAllocator,
                VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                tAsset->mVertexData.data(), sizeof(tAsset->mVertexData[0]) * tAsset->mVertexData.size());

            object->mMesh->mIndexBuffer = Renderer::CreateAndUploadBuffer(
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
                    object->mMaterial->mTextureAlbedo = Renderer::CreateAndUploadTexture(mRenderContext,
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
                    object->mMaterial->mTextureNormal = Renderer::CreateAndUploadTexture(mRenderContext,
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
                    object->mMaterial->mTextureSpecular = Renderer::CreateAndUploadTexture(mRenderContext,
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
                std::vector<VkDescriptorSetLayout> layouts = { mRootSignatureScene->mDescriptorSetLayouts[1] };
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
    renderPassInfo.renderPass = mRenderTargetScene->mPass;
    renderPassInfo.framebuffer = mRenderTargetScene->mFramebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapchain->mExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();


    static int frame = -1;

    frame++;

    if (frame <= 2)
    {
        Renderer::TransitionImageLayout(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool,
            mSwapchain->mImages[imageIndex],
            mSwapchain->mImageFormat, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL, 1, commandBuffers[imageIndex]);
    }
    else
    {
        Renderer::TransitionImageLayout(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool,
            mSwapchain->mImages[imageIndex],
            mSwapchain->mImageFormat, VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL, 1, commandBuffers[imageIndex]);
    }



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


        vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, this->mPipelines[object->mRenderState.stateID.value()].second->pipeline);
        std::vector<VkDescriptorSet> objectSets = { descriptorSetsSceneObjects[imageIndex], object->mMaterial->mDescriptorSet };
        vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, this->mPipelines[object->mRenderState.stateID.value()].second->mRootSignature.lock()->mPipelineLayout, 0, objectSets.size(), objectSets.data(), 0, nullptr);

        VkBuffer vertexBuffers[] = { object->mMesh->mVertexBuffer->mBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[imageIndex], object->mMesh->mIndexBuffer->mBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdPushConstants(
            commandBuffers[imageIndex],
            this->mPipelines[object->mRenderState.stateID.value()].second->mRootSignature.lock()->mPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(glm::mat4),
            &object->transform);

        vkCmdDrawIndexed(commandBuffers[imageIndex], static_cast<uint32_t>(object->mAsset->mIndices.size()), 1, 0, 0, 0);

    }

    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    Renderer::TransitionImageLayout(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool,
        mRenderTargetScene->mColorImages[0]->mImage, mRenderTargetScene->mColorImages[0]->mFormat,
        VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL, 1, commandBuffers[imageIndex]);

    Renderer::TransitionImageLayout(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool,
        mRenderTargetFinal->mColorImages[0]->mImage, mRenderTargetFinal->mColorImages[0]->mFormat,
        VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL, 1, commandBuffers[imageIndex]);


    // Start compute fullscreen pass
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline->computePipeline);
    vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_COMPUTE, mRootSignatureCompute->mPipelineLayout, 0, 1, &mComputeDataPostfx.descriptorset, 0, 0);

    glm::ivec2 dispatchSize = glm::ivec2(16, 16);
    vkCmdDispatch(commandBuffers[imageIndex],
        (mRenderTargetFinal->mWidth + dispatchSize.x - 1) / dispatchSize.x, // x dispatch
        (mRenderTargetFinal->mHeight + dispatchSize.y - 1) / dispatchSize.y, // y dispatch
        1); // z dispatch


    Renderer::TransitionImageLayout(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mRenderTargetFinal->mColorImages[0]->mImage, mRenderTargetFinal->mColorImages[0]->mFormat,
        VkImageLayout::VK_IMAGE_LAYOUT_GENERAL, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, commandBuffers[imageIndex]);

    Renderer::TransitionImageLayout(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mSwapchain->mImages[imageIndex], mSwapchain->mImageFormat,
        VkImageLayout::VK_IMAGE_LAYOUT_GENERAL, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, commandBuffers[imageIndex]);

	// Copy final image to swapchain
	{
		VkImageCopy copy{};

		copy.srcOffset = { 0, 0, 0 };
		copy.srcSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		copy.srcSubresource.mipLevel = 0;
		copy.srcSubresource.layerCount = 1;
		copy.srcSubresource.baseArrayLayer = 0;

		copy.dstOffset = { 0, 0, 0 };
		copy.extent.depth = 1;
		copy.extent.width = mSwapchain->mExtent.width;
		copy.extent.height = mSwapchain->mExtent.height;

		copy.dstSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		copy.dstSubresource.mipLevel = 0;
		copy.dstSubresource.layerCount = 1;
		copy.dstSubresource.baseArrayLayer = 0;


		vkCmdCopyImage(commandBuffers[imageIndex],
            mRenderTargetFinal->mColorImages[0]->mImage, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            mSwapchain->mImages[imageIndex], VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

		// Transition swapchain to present
		Renderer::TransitionImageLayout(mRenderContext->mDevice->mDevice, mQueueGraphics->mVkQueue, commandPool, mSwapchain->mImages[imageIndex], mSwapchain->mImageFormat, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, commandBuffers[imageIndex]);
	}

    ImGui::Begin("Lights");                          // Create a window called "Hello, world!" and append into it.
    for (uint32_t i = 0; i < aScene->GetLights().size(); ++i)
    {

        ImGui::SliderFloat3("Position", glm::value_ptr(aScene->GetLights()[i]->position), 0.0f, 10.0);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::Separator();
    }


    ImGui::End();

    // Rendering
    ImGui::Render();


    VkClearValue clearValuesImgui{};
    clearValuesImgui.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    ImDrawData* draw_data = ImGui::GetDrawData();
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = mSwapchain->mRenderPass;
        info.framebuffer = mSwapchain->mFramebuffers[imageIndex];
        info.renderArea.extent.width = mSwapchain->mExtent.width;
        info.renderArea.extent.height = mSwapchain->mExtent.height;
        info.clearValueCount = 1;
        info.pClearValues = &clearValuesImgui;
        vkCmdBeginRenderPass(commandBuffers[imageIndex], &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffers[imageIndex]);

    // Submit command buffer
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