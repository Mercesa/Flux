#pragma once

#include <stdexcept>
#include <cassert>
#include <vector>
#include <memory>
#include "Renderer/BufferGPU.h"
#include "Renderer/TextureVK.h"

#include "Renderer/Queue.h"
#include "Renderer/DescriptorPool.h"
#include "Renderer/GraphicsDevice.h"
#include "Renderer/RenderContext.h"

#include "Swapchain.h"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"
#include <iostream>
#include <optional>
#include <set>
#include "Renderer.h"

namespace Flux
{
    namespace Gfx
    {


	const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

    class Texture;
	class Renderer
	{
    public:
		static VkShaderModule CreateShaderModule(VkDevice aDevice, const std::vector<char>& code);

		static VkCommandBuffer BeginSingleTimeCommands(VkDevice aDevice, VkCommandPool aCmdPool);

		static void EndSingleTimeCommands(VkDevice aDevice, VkQueue aQueue, VkCommandBuffer aCommandBuffer, VkCommandPool aCmdPool);

		static void TransitionImageLayout(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

		static void CreateBuffer(VkDevice aDevice, VmaAllocator aAllocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage properties, VkBuffer& buffer, VmaAllocation& bufferMemory);

        // This function quickly copies a buffer on the GPU
        static void CopyBuffer(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBuffer commandBuffer = BeginSingleTimeCommands(aDevice, aCmdPool);

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            EndSingleTimeCommands(aDevice, aQueue, commandBuffer, aCmdPool);
        }

        static void CopyBufferToImage(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBuffer commandBuffer = BeginSingleTimeCommands(aDevice, aCmdPool);

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

            EndSingleTimeCommands(aDevice, aQueue, commandBuffer, aCmdPool);
        }

        static std::shared_ptr<Gfx::BufferGPU> CreateAndUploadBuffer(VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VmaAllocator aAllocator, VmaMemoryUsage aBufferUsage, VkBufferUsageFlags aBufferFlags, void* aData, size_t aDataSize);


        static std::shared_ptr<Texture> CreateAndUploadTexture(std::shared_ptr<RenderContext> aContext,
            VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VmaAllocator aAllocator,
            uint32_t aWidth, uint32_t aHeight, uint32_t aImgSize, uint32_t aAmountOfMips, unsigned char* aImageData,
            VkFormat aFormat) {

            std::shared_ptr<Texture> tReturnTexture = std::make_shared<Texture>();

            VkBuffer stagingBuffer;
            VmaAllocation stagingBufferMemory;
            CreateBuffer(aDevice, aAllocator, aImgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferMemory);


            void* data;
            vmaMapMemory(aAllocator, stagingBufferMemory, &data);
            memcpy(data, aImageData, static_cast<size_t>(aImgSize));
            vmaUnmapMemory(aAllocator, stagingBufferMemory);

            CreateImage(aContext, aWidth, aHeight, aFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, tReturnTexture->mImage, tReturnTexture->mAllocation, aAmountOfMips);

            TransitionImageLayout(aDevice, aQueue, aCmdPool, tReturnTexture->mImage, aFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aAmountOfMips);
            CopyBufferToImage(aDevice, aQueue, aCmdPool, stagingBuffer, tReturnTexture->mImage, aWidth, aHeight);



            vkDestroyBuffer(aDevice, stagingBuffer, nullptr);
            vmaFreeMemory(aAllocator, stagingBufferMemory);

            generateMipmaps(aContext, aDevice, aQueue, aCmdPool, tReturnTexture->mImage, VK_FORMAT_R8G8B8A8_SRGB, aWidth, aHeight, aAmountOfMips);

            tReturnTexture->mView = CreateImageView(aContext, tReturnTexture->mImage, aFormat, VK_IMAGE_ASPECT_COLOR_BIT, aAmountOfMips);

            return std::move(tReturnTexture);
        }

    public:

        void IdleDevice(Flux::Gfx::GraphicsDevice* const aDevice)
        {
            vkDeviceWaitIdle(aDevice->mDevice);
        }


		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};


		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
			for (const auto& availableFormat : availableFormats) {
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync) {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR && !vsync) {
					return availablePresentMode;
				}
				else
				{
					return VK_PRESENT_MODE_FIFO_KHR;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		static bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}

		static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR aSurface) {
			QueueFamilyIndices indices = findQueueFamilies(device, aSurface);

			bool extensionsSupported = CheckDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if (extensionsSupported) {
				SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, aSurface);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

			return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
		}

		static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* aWindow) {
			if (capabilities.currentExtent.width != UINT32_MAX) {
				return capabilities.currentExtent;
			}
			else {
				int width, height;
				glfwGetFramebufferSize(aWindow, &width, &height);

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(instance, debugMessenger, pAllocator);
			}
		}

		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else {
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

        bool checkValidationLayerSupport() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : Flux::Gfx::validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }

        bool HasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        static void PickPhysicalDevice(std::shared_ptr<RenderContext> aContext, VkSurfaceKHR aSurface)
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(aContext->instance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(aContext->instance, &deviceCount, devices.data());

            for (const auto& device : devices) {
                if (IsDeviceSuitable(device, aSurface)) {
                    aContext->mDevice->mPhysicalDevice = device;
                    break;
                }
            }

            if (aContext->mDevice->mPhysicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }
        }

        static void CreateLogicalDevice(std::shared_ptr<RenderContext> aContext, VkSurfaceKHR aSurface)
        {
            QueueFamilyIndices indices = findQueueFamilies(aContext->mDevice->mPhysicalDevice, aSurface);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures deviceFeatures{};
            deviceFeatures.samplerAnisotropy = VK_TRUE;

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (aContext->debugMode) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(aContext->mDevice->mPhysicalDevice, &createInfo, nullptr, &aContext->mDevice->mDevice) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }

            aContext->mDevice->queueFamilies = indices;

        }

        static std::vector<const char*> GetRequiredExtensions(bool aDebug) {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (aDebug) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        static bool CheckValidationLayerSupport() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }

        static void CreateSurface()
        {

        }

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR aSurface) {
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, aSurface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }

        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
        {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }

        static void SetupDebugMessenger(std::shared_ptr<RenderContext> aContext)
        {
            if (!aContext->debugMode) return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(aContext->instance, &createInfo, nullptr, &aContext->debugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }

        static void CreateImage(std::shared_ptr<RenderContext> aContext, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage properties, VkImage& image, VmaAllocation& imageMemory, uint32_t miplevels)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = miplevels;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = properties;

            vmaCreateImage(aContext->memoryAllocator, &imageInfo, &allocInfo, &image, &imageMemory, nullptr);
        }

        static VkImageView CreateImageView(std::shared_ptr<RenderContext> aContext, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, uint32_t miplevels) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectMask;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = miplevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(aContext->mDevice->mDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }

            return imageView;
        }

        static void generateMipmaps(std::shared_ptr<RenderContext> aContext,
            VkDevice aDevice, VkQueue aQueue, VkCommandPool aCmdPool, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
            // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(aContext->mDevice->mPhysicalDevice, imageFormat, &formatProperties);

            if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                throw std::runtime_error("texture image format does not support linear blitting!");
            }

            VkCommandBuffer commandBuffer = BeginSingleTimeCommands(aDevice, aCmdPool);

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                VkImageBlit blit{};
                blit.srcOffsets[0] = { 0, 0, 0 };
                blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = { 0, 0, 0 };
                blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vkCmdBlitImage(commandBuffer,
                    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            EndSingleTimeCommands(aContext->mDevice->mDevice, aQueue, commandBuffer, aCmdPool);
        }

        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR aSurface) {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, aSurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, aSurface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, aSurface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, aSurface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, aSurface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        static std::shared_ptr<Swapchain> CreateSwapChain(std::shared_ptr<RenderContext> aContext, GLFWwindow* window)
        {
            assert(aContext);

            std::shared_ptr<Swapchain> tSwapChain = std::make_shared<Swapchain>();

            if (aContext->surface == VK_NULL_HANDLE)
            {
                if (glfwCreateWindowSurface(aContext->instance, window, nullptr, &aContext->surface) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create window surface!");
                }
            }


            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(aContext->mDevice->mPhysicalDevice, aContext->surface);

            VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes, aContext->vsync);
            VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = aContext->surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices = findQueueFamilies(aContext->mDevice->mPhysicalDevice, aContext->surface);
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            if (vkCreateSwapchainKHR(aContext->mDevice->mDevice, &createInfo, nullptr, &tSwapChain->mSwapChain) != VK_SUCCESS) {
                throw std::runtime_error("failed to create swap chain!");
            }

            vkGetSwapchainImagesKHR(aContext->mDevice->mDevice, tSwapChain->mSwapChain, &imageCount, nullptr);
            tSwapChain->mImages.resize(imageCount);
            vkGetSwapchainImagesKHR(aContext->mDevice->mDevice, tSwapChain->mSwapChain, &imageCount, tSwapChain->mImages.data());

            tSwapChain->mImageFormat = surfaceFormat.format;
            tSwapChain->mExtent = extent;

            tSwapChain->mImageViews.resize(tSwapChain->mImages.size());

            for (size_t i = 0; i < tSwapChain->mImages.size(); i++) {
                tSwapChain->mImageViews[i] = Renderer::CreateImageView(aContext, tSwapChain->mImages[i], tSwapChain->mImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
            }

            VkFormat depthFormat = FindDepthFormat(aContext);
            Renderer::CreateImage(aContext, tSwapChain->mExtent.width, tSwapChain->mExtent.height, depthFormat,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
                tSwapChain->depthImage, tSwapChain->depthImageMemory, 1);
            tSwapChain->depthImageView = Renderer::CreateImageView(aContext, tSwapChain->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

            return tSwapChain;
        }

        static void DestroySwapchain(std::shared_ptr<RenderContext> aContext, std::shared_ptr<Swapchain> aSwapchain)
        {
            assert(aSwapchain);

            vmaFreeMemory(aContext->memoryAllocator, aSwapchain->depthImageMemory);
            vkDestroyImage(aContext->mDevice->mDevice, aSwapchain->depthImage, nullptr);
            vkDestroyImageView(aContext->mDevice->mDevice, aSwapchain->depthImageView, nullptr);

            for (auto imageView : aSwapchain->mImageViews) {
                vkDestroyImageView(aContext->mDevice->mDevice, imageView, nullptr);
            }

            vkDestroySwapchainKHR(aContext->mDevice->mDevice, aSwapchain->mSwapChain, nullptr);
            vkDestroySurfaceKHR(aContext->instance, aContext->surface, nullptr);
            aContext->surface = VK_NULL_HANDLE;

        }

        static std::shared_ptr<RenderContext> CreateRenderContext(std::string aName, bool aEnableDebug, GLFWwindow * aWindow)
        {
            std::shared_ptr<RenderContext> tRenderContext = std::make_shared<RenderContext>();
            tRenderContext->debugMode = true;

            tRenderContext->mDevice = std::make_shared<GraphicsDevice>();


            if (aEnableDebug && !CheckValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = aName.c_str();
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "Flux";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_2;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            auto extensions = GetRequiredExtensions(tRenderContext->debugMode);
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            if (tRenderContext->debugMode) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else {
                createInfo.enabledLayerCount = 0;

                createInfo.pNext = nullptr;
            }

            if (vkCreateInstance(&createInfo, nullptr, &tRenderContext->instance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }

            if (glfwCreateWindowSurface(tRenderContext->instance, aWindow, nullptr, &tRenderContext->surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }


            PickPhysicalDevice(tRenderContext, tRenderContext->surface);
            CreateLogicalDevice(tRenderContext, tRenderContext->surface);

            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.physicalDevice = tRenderContext->mDevice->mPhysicalDevice;
            allocatorInfo.device = tRenderContext->mDevice->mDevice;
            allocatorInfo.instance = tRenderContext->instance;
            allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;

            vmaCreateAllocator(&allocatorInfo, &tRenderContext->memoryAllocator);

            SetupDebugMessenger(tRenderContext);

            return tRenderContext;
        }

        static void DestroyRenderContext(std::shared_ptr<RenderContext> aRenderContext)
        {
            if (aRenderContext->debugMode) {
                DestroyDebugUtilsMessengerEXT(aRenderContext->instance, aRenderContext->debugMessenger, nullptr);
            }

            vmaDestroyAllocator(aRenderContext->memoryAllocator);
            //vkDestroySurfaceKHR(aRenderContext->instance, aRenderContext->surface, nullptr);
            vkDestroyDevice(aRenderContext->mDevice->mDevice, nullptr);

            vkDestroyInstance(aRenderContext->instance, nullptr);
        }

        static std::shared_ptr<Queue> CreateQueue(std::shared_ptr<RenderContext> aRenderContext, QueueCreateDesc* const aQueueDesc)
        {
            assert(aRenderContext);
            assert(aRenderContext->mDevice);

            std::shared_ptr<Queue> tQueue = std::make_shared<Queue>();

            switch (aQueueDesc->mType)
            {
            case eQueueType::QUEUE_TYPE_GRAPHICS:
                tQueue->mQueueIndex = aRenderContext->mDevice->queueFamilies.graphicsFamily.value();
                break;
            case eQueueType::QUEUE_TYPE_PRESENT:
                tQueue->mQueueIndex = aRenderContext->mDevice->queueFamilies.presentFamily.value();
                break;
            case eQueueType::QUEUE_TYPE_COMPUTE:
                assert(true); // not implemented yet
                break;
            case eQueueType::QUEUE_TYPE_TRANSFER:
                assert(true); // not implemented yet
                break;

            }

            vkGetDeviceQueue(aRenderContext->mDevice->mDevice, tQueue->mQueueIndex, 0, &tQueue->mVkQueue);

            return tQueue;
        }

        static void DestroyQueue(std::shared_ptr<QueueCreateDesc> aQueue)
        {

        }

        static std::shared_ptr<DescriptorPool> CreateDescriptorPool(std::shared_ptr<RenderContext> aRendererContext, DescriptorPoolCreateDesc* aPoolDesc)
        {
            std::shared_ptr<DescriptorPool> tDescriptorPool = std::make_shared<DescriptorPool>();

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
            descriptorPoolCreateInfo.maxSets = aPoolDesc->maxDescriptorSets;
            descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            if (vkCreateDescriptorPool(aRendererContext->mDevice->mDevice, &descriptorPoolCreateInfo, nullptr, &tDescriptorPool->mPool) != VK_SUCCESS)
            {
                throw std::runtime_error(" Failed to create descriptor pool");
            }

            return tDescriptorPool;
        }

        static void DestroyDescriptorPool(std::shared_ptr<RenderContext> aContext, std::shared_ptr<DescriptorPool> aPool)
        {
            vkDestroyDescriptorPool(aContext->mDevice->mDevice, aPool->mPool, nullptr);
        }

        static VkFormat FindSupportedFormat(std::shared_ptr<RenderContext> aContext, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
        {
            for (VkFormat format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(aContext->mDevice->mPhysicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                }
                else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }

            throw std::runtime_error("failed to find supported format!");
        }

        static VkFormat FindDepthFormat(std::shared_ptr<RenderContext> aContext) {
            return FindSupportedFormat(aContext,
                { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );


        }

	};
    }
};
