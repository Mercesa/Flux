#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vk_mem_alloc.h>

#include "Renderer/Reflection.h"
#include "Renderer/Renderer.h"

#include "Application/BasicGeometry.h"
#include "Common/AssetProcessing/AssetObjects.h"
#include "Application/Scene/iScene.h"

#include "Renderer/TextureVK.h"
#include "Application/Rendering/RenderingResourceManager.h"
const int MAX_FRAMES_IN_FLIGHT = 1;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace Flux
{
	class Camera;
	class SwapchainVK;
	class CustomRenderer
	{
	public:
		CustomRenderer(std::shared_ptr<Camera> aCamera);
		void Init();
		void WaitIdle();
		void Draw(const std::shared_ptr<iScene> aScene);
		void Cleanup();
		void SetWindow(GLFWwindow* aWindow);


		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct UniformBufferObject
		{
			glm::mat4 view;
			glm::mat4 projection;
		};

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else {
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(instance, debugMessenger, pAllocator);
			}
		}

		void Run() {
			InitVulkan();
			MainLoop();
			Cleanup();
		}



		bool mVsync;

		VmaAllocator memoryAllocator;

		VkImage textureImageCube;
		VmaAllocation textureImageMemoryCube;
		VkImage textureImageTriangle;
		VmaAllocation textureImageMemoryTriangle;

		VkSampler textureSampler;

		//BasicGeometry *cube;
		//BasicGeometry *triangle;
		//BasicGeometry *sphere;

		std::vector<VkBuffer> uniformBuffer;
		std::vector<VmaAllocation> uniformBufferMemory;

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout descriptorSetLayout;

		VkPipeline graphicsPipelineSceneObject;
		VkPipelineLayout pipelineLayoutSceneObjects;
		VkDescriptorSetLayout descriptorSetLayoutSceneObjects;
		std::vector<VkDescriptorSet> descriptorSetsSceneObjects;
		std::vector<VkBuffer> uniformBufferSceneObject;
		std::vector<VmaAllocation> uniformBufferMemorySceneObject;

		VkPipeline graphicsPipelineCube;
		VkPipeline graphicsPipelineTriangle;
		VkPipeline graphicsPipelineSphere;

		VkDescriptorPool descriptorPool;

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;

		std::vector<std::shared_ptr<BufferVK>> mSceneBuffers;
		std::vector<std::shared_ptr<TextureVK>> mSceneTextures;
		std::vector<std::shared_ptr<VkDescriptorSet>> mSceneSets;

		std::unique_ptr<RenderingResourceManager> mResourceManager;

		VkImageView textureImageViewCube;
		VkImageView textureImageViewTriangle;


		std::vector<std::shared_ptr<BufferVK>> mBuffers;

		bool framebufferResized = false;

		std::shared_ptr<Camera> mCamera;

		std::shared_ptr<Renderer> mRenderer;

	private:

		void InitVulkan();

		void MainLoop();

		void CleanupSwapChain();

		void RecreateSwapChain();

		void CreateInstance();

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void SetupDebugMessenger();

		void CreateSurface();

		void PickPhysicalDevice();

		void CreateLogicalDevice();

		void CreateSwapChain();

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

		void CreateImageViews();

		void CreateRenderPass();

		void CreateGraphicsPipelines();
		void CreateGraphicsPipelineSceneObject();
		void CreateGraphicsPipelineCube();
		void CreateGraphicsPipelineTriangle();
		void CreateGraphicsPipelineSphere();

		void CreateDescriptorSetLayout();

		void CreateFramebuffers();

		void CreateCommandPool();

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage properties, VkImage& image, VmaAllocation& imageMemory);


		void CreateDepthResources();

		void CreateTextureImages();
		void CreateTextureImageCube();
		void CreateTextureImageTriangle();

		void CreateTextureImageViews();

		void CreateTextureSampler();

		void CreateGeometry();

		void CreateUniformBuffers();

		void CreateDescriptorPool();

	    void CreateDescriptorSets();

		void CreateCommandBuffers();

		void CreateSyncObjects();

		void UpdateUniformBuffer(uint32_t currentImage);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		bool IsDeviceSuitable(VkPhysicalDevice device);

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		GLFWwindow* mWindow;

		std::vector<const char*> GetRequiredExtensions() {
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (enableValidationLayers) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			return extensions;
		}

		bool CheckValidationLayerSupport() {
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

		bool HasStencilComponent(VkFormat format) {
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
			for (VkFormat format : candidates) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
					return format;
				}
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
					return format;
				}
			}

			throw std::runtime_error("failed to find supported format!");
		}

		VkFormat FindDepthFormat() {
			return FindSupportedFormat(
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);


		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


		public:
		std::shared_ptr<SwapchainVK> mSwapchain;
	};

}


