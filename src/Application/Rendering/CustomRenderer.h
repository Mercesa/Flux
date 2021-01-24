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
#include "Renderer/TextureVK.h"
#include "Renderer/BufferGPU.h"
#include "Renderer/Queue.h"

#include "Application/BasicGeometry.h"
#include "Application/Scene/iScene.h"
#include "Application/Rendering/RenderState.h"

#include "Common/AssetProcessing/AssetObjects.h"

#include "Application/Rendering/RenderingResourceManager.h"
const int MAX_FRAMES_IN_FLIGHT = 1;



constexpr uint32_t AMOUNT_PREALLOCATED_OBJECTS = 0;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace Flux
{
	class Camera;
	class Swapchain;
	class CustomRenderer
	{
	public:
		CustomRenderer(GLFWwindow* window);
		void Init();
		void WaitIdle();
		void Draw(const std::shared_ptr<iScene> aScene);
		void Cleanup();
		void SetWindow(GLFWwindow* aWindow);

		struct UniformBufferCamera
		{
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec4 position;
			float nearPlane;
			float farPlane;
			float angle;
			float padding;
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

		struct UniformBufferDataCamera
		{


		};

		void Run() {
			InitVulkan();
			MainLoop();
			Cleanup();
		}

		bool mVsync;

		std::shared_ptr<Flux::Gfx::Texture> mEmptyTexture;

		VkSampler textureSampler;

		std::vector<VkBuffer> uniformBufferCameraBuffer;
		std::vector<VmaAllocation> uniformBufferCameraMemory;


		VkRenderPass renderPass;
		VkDescriptorSetLayout descriptorSetLayout;

		VkDescriptorSetLayout descriptorSetLayoutPerObject;

		VkPipelineLayout pipelineLayoutSceneObjects;
		VkDescriptorSetLayout descriptorSetLayoutSceneObjects;
		std::vector<VkDescriptorSet> descriptorSetsSceneObjects;


		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;

		std::vector<std::shared_ptr<Gfx::BufferGPU>> mSceneBuffers;
		std::vector<std::shared_ptr<Flux::Gfx::Texture>> mSceneTextures;
		std::vector<std::shared_ptr<VkDescriptorSet>> mSceneSets;
		std::vector<std::pair<RenderState, VkPipeline>> mPipelines;

		std::unique_ptr<RenderingResourceManager> mResourceManager;


		std::vector<std::shared_ptr<Gfx::BufferGPU>> mBuffers;

		bool framebufferResized = false;

		//std::shared_ptr<Renderer> mRenderer;

	private:

		void InitVulkan();

		void MainLoop();

		void CleanupSwapChain();

		void RecreateSwapChain();


		void CreateRenderPass();

		VkPipeline CreateGraphicsPipelineForState(RenderState state);
		std::optional<uint32_t> QueryPipeline(RenderState state);
		uint32_t CreatePipeline(RenderState state);

		void CreateDescriptorSetLayout();

		void CreateFramebuffers();

		void CreateCommandPool();

		void CreateTextureSampler();

		void CreateUniformBuffers();

	    void CreateDescriptorSets();

		void CreateCommandBuffers();

		void CreateSyncObjects();

		void UpdateUniformBuffer(uint32_t currentImage, std::shared_ptr<Camera> aCam);

		GLFWwindow* mWindow;

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


		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


		public:
		std::shared_ptr<Flux::Gfx::Renderer> mRenderer;
		std::shared_ptr<Flux::Gfx::RenderContext> mRenderContext;
		std::shared_ptr<Flux::Gfx::Swapchain> mSwapchain;
		std::shared_ptr<Flux::Gfx::Queue> mQueueGraphics;
		std::shared_ptr<Flux::Gfx::Queue> mQueuePresent;
		std::shared_ptr<Flux::Gfx::DescriptorPool> mDescriptorPool;


	};

}


