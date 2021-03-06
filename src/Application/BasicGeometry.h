//#pragma once
//
//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/glm.hpp>
//
//#include <vk_mem_alloc.h>
//
//#include <vector>
//
//namespace Flux
//{
//
//	class CustomRenderer;
//
//	class BasicGeometry
//	{
//		VkBuffer vertexBuffer;
//		VmaAllocation vertexBufferMemory;
//		VkBuffer indexBuffer;
//		VmaAllocation indexBufferMemory;
//
//	protected:
//		std::vector<VkDescriptorSet> descriptorSets;
//
//		std::vector<VkBuffer> uniformBuffer;
//		std::vector<VmaAllocation> uniformBufferMemory;
//
//		CustomRenderer*renderer;
//		VkDescriptorPool mDescriptorPool;
//
//		BasicGeometry(CustomRenderer*renderer, VkDescriptorPool aPool);
//
//		void CreateIndexBuffer(void *data, size_t dataSize);
//		void CreateVertexBuffer(void *data, size_t dataSize);
//
//	public:
//		virtual ~BasicGeometry();
//
//		void CreateUniformBuffers();
//		virtual void CreateDescriptorSets() = 0;
//
//		VkBuffer GetVertexBuffer() { return vertexBuffer; }
//		VkBuffer GetIndexBuffer() { return indexBuffer; }
//		VkDescriptorSet *GetDescriptorSet(size_t i) { return &descriptorSets[i]; }
//
//		void SetModelTransform(glm::mat4 const &aModelMat, uint32_t currentImage);
//
//		virtual VkVertexInputBindingDescription GetBindingDescription() = 0;
//		virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() = 0;
//		virtual uint32_t GetIndexCount() = 0;
//	};
//
//	class Cube : public BasicGeometry
//	{
//		struct Vertex {
//			glm::vec3 pos;
//			glm::vec2 texCoord;
//		};
//
//		uint32_t numIndices;
//
//	public:
//		Cube(CustomRenderer*renderer, VkDescriptorPool aPool);
//
//		virtual void CreateDescriptorSets() override;
//		virtual VkVertexInputBindingDescription GetBindingDescription() override;
//		virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() override;
//		virtual uint32_t GetIndexCount() override;
//	};
//
//	class Triangle : public BasicGeometry
//	{
//		struct Vertex {
//			glm::vec3 pos;
//			glm::vec3 colour;
//			glm::vec2 texCoord;
//		};
//
//	public:
//		Triangle(CustomRenderer*renderer, VkDescriptorPool aPool);
//
//		virtual void CreateDescriptorSets() override;
//		virtual VkVertexInputBindingDescription GetBindingDescription() override;
//		virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() override;
//		virtual uint32_t GetIndexCount() override;
//	};
//
//	class Sphere : public BasicGeometry
//	{
//		struct Vertex {
//			glm::vec3 pos;
//			glm::vec3 normal;
//		};
//
//		uint32_t numIndices;
//
//	public:
//		Sphere(CustomRenderer*renderer, VkDescriptorPool aPool);
//
//		virtual void CreateDescriptorSets() override;
//		virtual VkVertexInputBindingDescription GetBindingDescription() override;
//		virtual std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() override;
//		virtual uint32_t GetIndexCount() override;
//	};
//
//}
