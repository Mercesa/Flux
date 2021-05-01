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

		struct RenderDataQuery
		{
			std::vector<uint64_t> pipelineStats;
			std::vector<std::string> pipelineStatNames
			{
				"Input assembly vertex count        ",
				"Input assembly primitives count        ",
				"Vertex shader invocation        ",
				"Clipping stage primitives processed        ",
				"Clipping stage primitives output        ",
				"Fragment shader invocations        ",
				"Tess. control shader patches        ",
				"Tess. eval. shader invocations        "
			};
		} mRenderDataPipeline;

		void GetQueryResults()
		{
			// We use vkGetQueryResults to copy the results into a host visible buffer
			vkGetQueryPoolResults(
				mRenderContext->mDevice->mDevice,
				mQueryPool,
				0,
				1,
				sizeof(uint64_t) * mRenderDataPipeline.pipelineStats.size(),
				mRenderDataPipeline.pipelineStats.data(),
				sizeof(uint64_t),
				// Store results a 64 bit values and wait until the results have been finished
				// If you don't want to wait, you can use VK_QUERY_RESULT_WITH_AVAILABILITY_BIT
				// which also returns the state of the result (ready) in the result
				VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
		}

		LightData mLightData;

		struct ComputeDataPostfx
		{
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
		VkSampler pointSampler;



		std::shared_ptr<Flux::Gfx::RootSignature> mRootSignatureSceneObjects;

		//std::vector<std::shared_ptr<Flux::Gfx::Shader>>;
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
		std::vector<std::pair<RenderState, std::shared_ptr<Flux::Gfx::GraphicsPipeline>>> mPipelines;


		std::unique_ptr<RenderingResourceManager> mResourceManager;

		struct VertexPosUv
		{
			glm::vec3 position;
			glm::vec2 texCoords;
			glm::vec3 normal;
			glm::vec3 bitangent;
			glm::vec3 tangent;
		};

		bool framebufferResized = false;

	private:
		std::optional<std::shared_ptr<Flux::Gfx::Shader>> DoesShaderExist(std::string aFilePath);
		std::optional<std::shared_ptr<Flux::Gfx::RootSignature>> DoesRootSignatureExist(const std::vector<std::shared_ptr<Flux::Gfx::Shader>>& aShaders);


		void InitVulkan();

		void SetupQueryPool();

		void MainLoop();

		void CleanupSwapChain();

		void RecreateSwapChain();

		std::shared_ptr<Flux::Gfx::GraphicsPipeline> CreateGraphicsPipelineForState(RenderState state);
		std::optional<uint32_t> QueryPipeline(RenderState state);
		uint32_t CreatePipeline(RenderState state);

		void CreateCommandPool();

		void CreateTextureSampler();

		void CreateUniformBuffers();

		void CreateDescriptorSets();

		void CreateCommandBuffers();

		void CreateSyncObjects();

		void UpdateUniformBuffer(uint32_t currentImage, std::shared_ptr<Camera> aCam, std::vector<std::shared_ptr<Light>> aLights);

		GLFWwindow* mWindow;

	public:
		std::vector < std::shared_ptr<Flux::Gfx::Shader>> mShadersAll;
		std::vector<std::shared_ptr<Flux::Gfx::RootSignature>> mRootSignaturesAll;

		std::shared_ptr<Flux::Gfx::RenderContext> mRenderContext;
		std::shared_ptr<Flux::Gfx::Swapchain> mSwapchain;
		std::shared_ptr<Flux::Gfx::Queue> mQueueGraphics;
		std::shared_ptr<Flux::Gfx::Queue> mQueuePresent;
		std::shared_ptr<Flux::Gfx::DescriptorPool> mDescriptorPool;

		std::shared_ptr<Flux::Gfx::RenderTarget> mRenderTargetScene;

		std::shared_ptr<Flux::Gfx::RenderTarget> mRenderTargetFinal;
		std::shared_ptr<Flux::Gfx::RootSignature> mRootSignatureCompute;
		std::shared_ptr<Flux::Gfx::Shader> mComputeShader;
		std::shared_ptr<Flux::Gfx::ComputePipeline> mComputePipeline;


		std::shared_ptr<Gfx::RootSignature> mRootSignatureScene;


		struct DepthOnlyPass
		{
			std::shared_ptr<Gfx::RootSignature> mRootSignatureDepthOnly;
			std::shared_ptr<Gfx::GraphicsPipeline> mGraphicsPipeline;
			std::shared_ptr<Gfx::RenderTarget> mRenderTargetDepth;
			std::vector<std::shared_ptr<Flux::Gfx::BufferGPU>> mBufferDepthTransformation;
			std::vector<VkDescriptorSet> descriptorSet;
			std::vector<VkDescriptorSet> descriptorSetShadowTexture;


		}mDepthOnlypass;


		VkQueryPool mQueryPool;

	};

}


