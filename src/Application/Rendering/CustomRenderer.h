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

#include "Renderer/Renderer.h"
#include "Renderer/TextureVK.h"
#include "Renderer/BufferGPU.h"
#include "Renderer/Queue.h"

#include "Application/BasicGeometry.h"
#include "Application/Scene/iScene.h"
#include "Application/Rendering/RenderState.h"

#include "Common/AssetProcessing/AssetObjects.h"

#include "Application/Rendering/RenderingResourceManager.h"
#include "Application/Rendering/RenderDataStructs.h"


constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr int AMOUNT_OF_SUPPORTED_LIGHTS = 1024;
constexpr bool FORCE_DEBUG = true;


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

		struct UniformBufferLights
		{
			Light lights[AMOUNT_OF_SUPPORTED_LIGHTS];
		};

		struct LightData
		{
			std::vector<std::shared_ptr<Gfx::BufferGPU>> mUniformBuffersLights;
			std::vector<VkDescriptorSet> mSets;
			Light lightCache[AMOUNT_OF_SUPPORTED_LIGHTS];
		};

		LightData mLightData;

		struct ComputeDataPostfx
		{
			VkPipeline pipeline;
			VkDescriptorSet descriptorset;

		} mComputeDataPostfx;

		void Run() {
			InitVulkan();
			MainLoop();
			Cleanup();
		}

		bool mVsync;

		std::shared_ptr<Flux::Gfx::Texture> mEmptyTexture;

		VkSampler textureSampler;



		std::shared_ptr<Flux::Gfx::RootSignature> mRootSignatureSceneObjects;

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


		std::vector<std::shared_ptr<Gfx::BufferGPU>> mUniformBuffersCamera;
		std::vector<std::shared_ptr<Gfx::BufferGPU>> mSceneBuffers;
		std::vector<std::shared_ptr<Flux::Gfx::Texture>> mSceneTextures;


		std::vector<std::shared_ptr<VkDescriptorSet>> mSceneSets;
		std::vector<std::pair<RenderState, VkPipeline>> mPipelines;


		std::unique_ptr<RenderingResourceManager> mResourceManager;

		struct VertexPosUv
		{
			glm::vec3 position;
			glm::vec2 texCoords;
			glm::vec3 normal;
			glm::vec3 bitangent;
			glm::vec3 tangent;
		};



		//struct Quad
		//{
		//	std::vector<VertexPosUv> fsQuadVertices
		//	{
		//		{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f)}, // top left
		//		{glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)}, // top right
		//		{glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}, // bottom left
		//		{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)}, // bottom right
		//	};

		//	std::vector<uint32_t> fsQuadIndices
		//	{
		//		2, 1, 0, 3, 1, 2
		//	};

		//	std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributes()
		//	{
		//		std::vector<VkVertexInputAttributeDescription> vertexAttrDescriptions;
		//		vertexAttrDescriptions.resize(5);

		//		vertexAttrDescriptions[0].binding = 0;
		//		vertexAttrDescriptions[0].location = 0;
		//		vertexAttrDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		//		vertexAttrDescriptions[0].offset = offsetof(VertexData, position);

		//		vertexAttrDescriptions[1].binding = 0;
		//		vertexAttrDescriptions[1].location = 1;
		//		vertexAttrDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		//		vertexAttrDescriptions[1].offset = offsetof(VertexData, texCoords);

		//		vertexAttrDescriptions[2].binding = 0;
		//		vertexAttrDescriptions[2].location = 2;
		//		vertexAttrDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		//		vertexAttrDescriptions[2].offset = offsetof(VertexData, normal);

		//		vertexAttrDescriptions[3].binding = 0;
		//		vertexAttrDescriptions[3].location = 3;
		//		vertexAttrDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		//		vertexAttrDescriptions[3].offset = offsetof(VertexData, tangent);

		//		vertexAttrDescriptions[4].binding = 0;
		//		vertexAttrDescriptions[4].location = 4;
		//		vertexAttrDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		//		vertexAttrDescriptions[4].offset = offsetof(VertexData, bitangent);

		//		return vertexAttrDescriptions;
		//	}

		//	std::shared_ptr<Gfx::BufferGPU> vertexBuffer;
		//	std::shared_ptr<Gfx::BufferGPU> indexBuffer;

		//	VkPipeline pipeline;
		//	VkPipelineLayout pipelineLayout;
		//	VkDescriptorSetLayout descriptorSetLayout;
		//	VkDescriptorSet descriptorSet;
		//} fsQuad;

		bool framebufferResized = false;

	private:

		void InitVulkan();

		void MainLoop();

		void CleanupSwapChain();

		void RecreateSwapChain();

		VkPipeline CreateGraphicsPipelineForState(RenderState state);
		std::optional<uint32_t> QueryPipeline(RenderState state);
		uint32_t CreatePipeline(RenderState state);

		void CreateDescriptorSetLayout();

		void CreateCommandPool();

		void CreateTextureSampler();

		void CreateUniformBuffers();

	    void CreateDescriptorSets();

		void CreateCommandBuffers();

		void CreateSyncObjects();

		void UpdateUniformBuffer(uint32_t currentImage, std::shared_ptr<Camera> aCam, std::vector<std::shared_ptr<Light>> aLights);

		GLFWwindow* mWindow;



		public:
		std::shared_ptr<Flux::Gfx::RenderContext> mRenderContext;
		std::shared_ptr<Flux::Gfx::Swapchain> mSwapchain;
		std::shared_ptr<Flux::Gfx::Queue> mQueueGraphics;
		std::shared_ptr<Flux::Gfx::Queue> mQueuePresent;
		std::shared_ptr<Flux::Gfx::DescriptorPool> mDescriptorPool;

		std::shared_ptr<Flux::Gfx::RenderTarget> mRenderTargetScene;

		std::shared_ptr<Flux::Gfx::RenderTarget> mRenderTargetFinal;
		std::shared_ptr<Flux::Gfx::RootSignature> mRootSignatureCompute;
		std::shared_ptr<Flux::Gfx::Shader> mComputeShader;


	};

}


