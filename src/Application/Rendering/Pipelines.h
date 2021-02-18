#pragma once

#include "Renderer/Renderer.h"
#include "Common/AssetProcessing/AssetObjects.h"
#include "Common/FileHandling/FileReadUtility.h"

#include "Application/Rendering/RenderState.h"

static VkCullModeFlagBits ConvertCullModeToVkCullBit(Flux::CullModes aType)
{
    switch (aType)
    {
    case Flux::CullModes::eCullFront:
        return VK_CULL_MODE_FRONT_BIT;
        break;
    case Flux::CullModes::eCullBack:
        return VK_CULL_MODE_BACK_BIT;
        break;
    case Flux::CullModes::eCullFrontAndBack:
        return VK_CULL_MODE_FRONT_AND_BACK;
        break;

    default:
        VkCullModeFlagBits(0);
    }
}

static VkFrontFace ConvertFrontFaceToVkFaceBit(Flux::FrontFace aType)
{
    switch (aType)
    {
    case Flux::FrontFace::eCounterClockWise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    case Flux::FrontFace::eClockWise:
        return VK_FRONT_FACE_CLOCKWISE;
        break;

    default:
        VkCullModeFlagBits(0);
    }
}

VkShaderStageFlagBits ConvertShaderToVkShaderStageBit(Flux::ShaderTypes aType)
{
    switch (aType)
    {
    case Flux::ShaderTypes::eVertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case Flux::ShaderTypes::eFragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
        break;

    default:
        VkShaderStageFlagBits(0);
    }
}

static VkPipeline CustomRendererCreateGraphicsPipelineForState(
    Flux::RenderState aRenderState,
    std::shared_ptr<Flux::Gfx::RenderContext> aContext,
    VkViewport aViewport, VkRect2D aRect,
    VkPipelineLayout aPipelineLayout,
    VkRenderPass aRenderPass,
    std::vector<VkVertexInputAttributeDescription> aVertexAttributes)
{
    std::vector<VkShaderModule> modules;
    std::vector<VkPipelineShaderStageCreateInfo> pipelineCreateInfo;

    // Prepare the pipeline stages
    for (auto& e : aRenderState.shaders)
    {
        auto shaderCode = Flux::Common::ReadFile(e.second);

        VkShaderModule shaderModule = Flux::Gfx::Renderer::CreateShaderModule(aContext->mDevice->mDevice, shaderCode);

        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = ConvertShaderToVkShaderStageBit(e.first);
        shaderStageInfo.module = shaderModule;
        shaderStageInfo.pName = "main";

        modules.push_back(shaderModule);
        pipelineCreateInfo.push_back(shaderStageInfo);
    }


    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Flux::VertexData);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(aVertexAttributes.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = aVertexAttributes.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &aViewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &aRect;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = ConvertCullModeToVkCullBit(aRenderState.drawState.cullMode);
    rasterizer.frontFace = ConvertFrontFaceToVkFaceBit(aRenderState.drawState.frontFaceMode);
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



    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
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
    pipelineInfo.layout = aPipelineLayout;
    pipelineInfo.renderPass = aRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline tPipeline;
    if (vkCreateGraphicsPipelines(aContext->mDevice->mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &tPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    for (auto& shaderModule : modules)
    {
        vkDestroyShaderModule(aContext->mDevice->mDevice, shaderModule, nullptr);
    }

    return tPipeline;
}