#pragma once

#include <iostream>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

namespace Flux
{
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    static ImGui_ImplVulkanH_Window wd{};

    //static void InitImgui(
    //    GLFWwindow* aWindow,
    //    VkInstance aInstance,
    //    VkPhysicalDevice aPhysicalDevice,
    //    VkDevice aDevice,
    //    uint32_t aVkQueueFamily,
    //    VkQueue aQueue,
    //    VkDescriptorPool aDescriptorPool,
    //    uint32_t imageCount
    //)
    //{
    //    // Setup Dear ImGui context
    //    IMGUI_CHECKVERSION();
    //    ImGui::CreateContext();
    //    ImGuiIO& io = ImGui::GetIO(); (void)io;

    //    ImGui::StyleColorsDark();

    //    ImGui_ImplGlfw_InitForVulkan(aWindow, true);
    //    ImGui_ImplVulkan_InitInfo init_info = {};
    //    init_info.Instance = aInstance;
    //    init_info.PhysicalDevice = aPhysicalDevice;
    //    init_info.Device = aDevice;
    //    init_info.QueueFamily = aVkQueueFamily;
    //    init_info.Queue = aQueue;
    //    init_info.PipelineCache = VK_NULL_HANDLE;
    //    init_info.DescriptorPool = aDescriptorPool;
    //    init_info.Allocator = nullptr;
    //    init_info.MinImageCount = imageCount;
    //    init_info.ImageCount = imageCount;
    //    init_info.CheckVkResultFn = check_vk_result;

    //    ImGui_ImplVulkanH_CreateOrResizeWindow(aInstance, aPhysicalDevice, aDevice, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);

    //    ImGui_ImplVulkan_Init(&init_info, wd.RenderPass);

    //    VkResult err;
    //    {
    //        // Use any command queue
    //        VkCommandPool command_pool = wd.Frames[wd.FrameIndex].CommandPool;
    //        VkCommandBuffer command_buffer = wd.Frames[wd.FrameIndex].CommandBuffer;

    //        err = vkResetCommandPool(aDevice, command_pool, 0);
    //        check_vk_result(err);
    //        VkCommandBufferBeginInfo begin_info = {};
    //        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //        err = vkBeginCommandBuffer(command_buffer, &begin_info);
    //        check_vk_result(err);

    //        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    //        VkSubmitInfo end_info = {};
    //        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //        end_info.commandBufferCount = 1;
    //        end_info.pCommandBuffers = &command_buffer;
    //        err = vkEndCommandBuffer(command_buffer);
    //        check_vk_result(err);
    //        err = vkQueueSubmit(aQueue, 1, &end_info, VK_NULL_HANDLE);
    //        check_vk_result(err);

    //        err = vkDeviceWaitIdle(aDevice);
    //        check_vk_result(err);
    //        ImGui_ImplVulkan_DestroyFontUploadObjects();
    //    }
    //}


    void DestroyImgui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void NewFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Render(VkCommandBuffer buffer)
    {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        ImGui_ImplVulkanH_Frame* fd = &wd.Frames[wd.FrameIndex];

        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd.RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = wd.Width;
            info.renderArea.extent.height = wd.Height;
            info.clearValueCount = 1;
            info.pClearValues = &wd.ClearValue;
            vkCmdBeginRenderPass(buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, buffer);

        // Submit command buffer
        vkCmdEndRenderPass(buffer);
    }
}

