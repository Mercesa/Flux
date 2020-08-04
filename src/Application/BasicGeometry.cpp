#include "Application/BasicGeometry.h"
#include "Application/Rendering/CustomRenderer.h"
#include "Renderer/SwapchainVK.h"
#define M_PI 3.14159265358979323846

using namespace Flux;
Flux::BasicGeometry::BasicGeometry(CustomRenderer *renderer, VkDescriptorPool aPool)
	: renderer(renderer), mDescriptorPool(aPool)
{
}

Flux::BasicGeometry::~BasicGeometry()
{
	vkDestroyBuffer(renderer->device, indexBuffer, nullptr);
	vmaFreeMemory(renderer->memoryAllocator, indexBufferMemory);

	vkDestroyBuffer(renderer->device, vertexBuffer, nullptr);
	vmaFreeMemory(renderer->memoryAllocator, vertexBufferMemory);

	for (size_t i = 0; i < renderer->mSwapchain->mImages.size(); i++) {
		vkDestroyBuffer(renderer->device, uniformBuffer[i], nullptr);
		vmaFreeMemory(renderer->memoryAllocator, uniformBufferMemory[i]);
	}
}

void Flux::BasicGeometry::CreateIndexBuffer(void *aData, size_t aDataSize)
{
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	renderer->CreateBuffer(aDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferMemory);

	void* data;
	vmaMapMemory(renderer->memoryAllocator, stagingBufferMemory, &data);
	memcpy(data, aData, aDataSize);
	vmaUnmapMemory(renderer->memoryAllocator, stagingBufferMemory);

	renderer->CreateBuffer(aDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,  VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, indexBuffer, indexBufferMemory);

	renderer->CopyBuffer(stagingBuffer, indexBuffer, aDataSize);

	vkDestroyBuffer(renderer->device, stagingBuffer, nullptr);
	vmaFreeMemory(renderer->memoryAllocator, stagingBufferMemory);
}

void Flux::BasicGeometry::CreateVertexBuffer(void *aData, size_t aDataSize)
{
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	renderer->CreateBuffer(aDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingBufferMemory);

	void* data;
	vmaMapMemory(renderer->memoryAllocator, stagingBufferMemory, &data);
	memcpy(data, aData, aDataSize);
	vmaUnmapMemory(renderer->memoryAllocator, stagingBufferMemory);

	renderer->CreateBuffer(aDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, vertexBuffer, vertexBufferMemory);

	renderer->CopyBuffer(stagingBuffer, vertexBuffer, aDataSize);

	vkDestroyBuffer(renderer->device, stagingBuffer, nullptr);
	vmaFreeMemory(renderer->memoryAllocator, stagingBufferMemory);
}

void Flux::BasicGeometry::CreateUniformBuffers()
{
	uniformBuffer.resize(renderer->mSwapchain->mImages.size());
	uniformBufferMemory.resize(renderer->mSwapchain->mImages.size());
	for (size_t i = 0; i < uniformBuffer.size(); i++)
	{
		renderer->CreateBuffer(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffer[i], uniformBufferMemory[i]);
	}
}

void Flux::BasicGeometry::SetModelTransform(glm::mat4 const &aModelMat, uint32_t currentImage)
{
	void *data;
	vmaMapMemory(renderer->memoryAllocator, uniformBufferMemory[currentImage], &data);
    memcpy(data, &aModelMat, sizeof(aModelMat));
    vmaUnmapMemory(renderer->memoryAllocator, uniformBufferMemory[currentImage]);
}

Flux::Cube::Cube(CustomRenderer *renderer, VkDescriptorPool aPool)
	: BasicGeometry(renderer, aPool)
{
	static std::vector<glm::vec3> const vertexPositions = {
		{ -0.5f,  0.5f, -0.5f },
		{  0.5f,  0.5f, -0.5f },
		{  0.5f,  0.5f,  0.5f },
		{ -0.5f,  0.5f,  0.5f },
		{ -0.5f, -0.5f, -0.5f },
		{  0.5f, -0.5f, -0.5f },
		{  0.5f, -0.5f,  0.5f },
		{ -0.5f, -0.5f,  0.5f }
	};
	static std::vector<uint16_t> const positionIndices = {
		0, 1, 2, 2, 3, 0,
		4, 7, 6, 6, 5, 4,
		4, 5, 1, 1, 0, 4,
		5, 6, 2, 2, 1, 5,
		6, 7, 3, 3, 2, 6,
		7, 4, 0, 0, 3, 7
	};
	static std::vector<glm::vec2> const faceUVs = {
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f }
	};
	static std::vector<uint16_t> const faceIndices = { 0, 1, 2, 2, 3, 0 };

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	for (unsigned faceIndex = 0; faceIndex < 6; faceIndex++) {
		unsigned baseIndex = faceIndex * 6;
		for (unsigned vertexIndex = 0; vertexIndex < 6; vertexIndex++) {
			vertices.push_back({
				vertexPositions[positionIndices[baseIndex + vertexIndex]],
				faceUVs[faceIndices[vertexIndex]]
			});
			indices.push_back(static_cast<uint16_t>(indices.size()));
		}
	}
	numIndices = static_cast<uint32_t>(indices.size());

	CreateIndexBuffer((void *)indices.data(), sizeof(indices[0]) * indices.size());
	CreateVertexBuffer((void *)vertices.data(), sizeof(vertices[0]) * vertices.size());
}

void Flux::Cube::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(renderer->mSwapchain->mImages.size(), renderer->descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(renderer->mSwapchain->mImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(renderer->mSwapchain->mImages.size());
	if (vkAllocateDescriptorSets(renderer->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allcoate descriptor sets!");
	}

	for (size_t i = 0; i < renderer->mSwapchain->mImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfoCamera{};
		bufferInfoCamera.buffer = renderer->uniformBuffer[i];
		bufferInfoCamera.offset = 0;
		bufferInfoCamera.range = sizeof(CustomRenderer::UniformBufferObject);

		VkDescriptorBufferInfo bufferInfoModel{};
		bufferInfoModel.buffer = uniformBuffer[i];
		bufferInfoModel.offset = 0;
		bufferInfoModel.range = sizeof(glm::mat4);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = renderer->textureImageViewCube;
		imageInfo.sampler = renderer->textureSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfoCamera;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfoModel;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(renderer->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

VkVertexInputBindingDescription Flux::Cube::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Flux::Cube::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> vertexAttrDescriptions;
	vertexAttrDescriptions.resize(2);

	vertexAttrDescriptions[0].binding = 0;
	vertexAttrDescriptions[0].location = 0;
	vertexAttrDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttrDescriptions[0].offset = offsetof(Vertex, pos);

	vertexAttrDescriptions[1].binding = 0;
	vertexAttrDescriptions[1].location = 1;
	vertexAttrDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttrDescriptions[1].offset = offsetof(Vertex, texCoord);

	return vertexAttrDescriptions;
}

uint32_t Flux::Cube::GetIndexCount()
{
	return numIndices;
}


Flux::Triangle::Triangle(CustomRenderer *renderer, VkDescriptorPool aPool)
	: BasicGeometry(renderer, aPool)
{
	static std::vector<Vertex> const vertices = {
		{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
		{ {  1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ {  0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f } }
	};
	static std::vector<uint16_t> const indices = { 0, 1, 2 };

	CreateIndexBuffer((void *)indices.data(), sizeof(indices[0]) * indices.size());
	CreateVertexBuffer((void *)vertices.data(), sizeof(vertices[0]) * vertices.size());
}

void Flux::Triangle::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(renderer->mSwapchain->mImages.size(), renderer->descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(renderer->mSwapchain->mImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(renderer->mSwapchain->mImages.size());
	if (vkAllocateDescriptorSets(renderer->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allcoate descriptor sets!");
	}

	for (size_t i = 0; i < renderer->mSwapchain->mImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfoCamera{};
		bufferInfoCamera.buffer = renderer->uniformBuffer[i];
		bufferInfoCamera.offset = 0;
		bufferInfoCamera.range = sizeof(CustomRenderer::UniformBufferObject);

		VkDescriptorBufferInfo bufferInfoModel{};
		bufferInfoModel.buffer = uniformBuffer[i];
		bufferInfoModel.offset = 0;
		bufferInfoModel.range = sizeof(glm::mat4);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = renderer->textureImageViewTriangle;
		imageInfo.sampler = renderer->textureSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfoCamera;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfoModel;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(renderer->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

VkVertexInputBindingDescription Flux::Triangle::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Flux::Triangle::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> vertexAttrDescriptions;
	vertexAttrDescriptions.resize(3);

	vertexAttrDescriptions[0].binding = 0;
	vertexAttrDescriptions[0].location = 0;
	vertexAttrDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttrDescriptions[0].offset = offsetof(Vertex, pos);

	vertexAttrDescriptions[1].binding = 0;
	vertexAttrDescriptions[1].location = 1;
	vertexAttrDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttrDescriptions[1].offset = offsetof(Vertex, colour);

	vertexAttrDescriptions[2].binding = 0;
	vertexAttrDescriptions[2].location = 2;
	vertexAttrDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttrDescriptions[2].offset = offsetof(Vertex, texCoord);

	return vertexAttrDescriptions;
}

uint32_t Flux::Triangle::GetIndexCount()
{
	return 3;
}


Flux::Sphere::Sphere(CustomRenderer *renderer, VkDescriptorPool aPool)
	: BasicGeometry(renderer, aPool)
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	static uint32_t constexpr parallels = 24;
	static uint32_t constexpr meridians = 24;
	vertices.push_back({{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
	for (uint32_t j = 0; j < parallels - 1; ++j)
	{
		double const polar = M_PI * double(j+1) / double(parallels);
		double const sp = std::sin(polar);
		double const cp = std::cos(polar);
		for (uint32_t i = 0; i < meridians; ++i)
		{
			double const azimuth = 2.0 * M_PI * double(i) / double(meridians);
			double const sa = std::sin(azimuth);
			double const ca = std::cos(azimuth);
			double const x = sp * ca;
			double const y = cp;
			double const z = sp * sa;
			vertices.push_back({{x, y, z}, glm::normalize(glm::vec3(x, y, z))});
		}
	}
	vertices.push_back({{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}});

	for (uint32_t i = 0; i < meridians; ++i)
	{
		uint32_t const a = i + 1;
		uint32_t const b = (i + 1) % meridians + 1;
		indices.push_back(0);
		indices.push_back(b);
		indices.push_back(a);
	}

	for (uint32_t j = 0; j < parallels - 2; ++j)
	{
		uint32_t aStart = j * meridians + 1;
		uint32_t bStart = (j + 1) * meridians + 1;
		for (uint32_t i = 0; i < meridians; ++i)
		{
			const uint32_t a = aStart + i;
			const uint32_t a1 = aStart + (i + 1) % meridians;
			const uint32_t b = bStart + i;
			const uint32_t b1 = bStart + (i + 1) % meridians;
			indices.push_back(a);
			indices.push_back(a1);
			indices.push_back(b1);
			indices.push_back(a);
			indices.push_back(b1);
			indices.push_back(b);
		}
	}

	for (uint32_t i = 0; i < meridians; ++i)
	{
		uint32_t const a = i + meridians * (parallels - 2) + 1;
		uint32_t const b = (i + 1) % meridians + meridians * (parallels - 2) + 1;
		indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
		indices.push_back(a);
		indices.push_back(b);
	}

	numIndices = static_cast<uint32_t>(indices.size());

	CreateIndexBuffer((void *)indices.data(), sizeof(indices[0]) * indices.size());
	CreateVertexBuffer((void *)vertices.data(), sizeof(vertices[0]) * vertices.size());
}

void Flux::Sphere::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(renderer->mSwapchain->mImages.size(), renderer->descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(renderer->mSwapchain->mImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(renderer->mSwapchain->mImages.size());
	if (vkAllocateDescriptorSets(renderer->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allcoate descriptor sets!");
	}

	for (size_t i = 0; i < renderer->mSwapchain->mImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfoCamera{};
		bufferInfoCamera.buffer = renderer->uniformBuffer[i];
		bufferInfoCamera.offset = 0;
		bufferInfoCamera.range = sizeof(CustomRenderer::UniformBufferObject);

		VkDescriptorBufferInfo bufferInfoModel{};
		bufferInfoModel.buffer = uniformBuffer[i];
		bufferInfoModel.offset = 0;
		bufferInfoModel.range = sizeof(glm::mat4);

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfoCamera;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfoModel;

		vkUpdateDescriptorSets(renderer->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

VkVertexInputBindingDescription Flux::Sphere::GetBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Flux::Sphere::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> vertexAttrDescriptions;
	vertexAttrDescriptions.resize(2);

	vertexAttrDescriptions[0].binding = 0;
	vertexAttrDescriptions[0].location = 0;
	vertexAttrDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttrDescriptions[0].offset = offsetof(Vertex, pos);

	vertexAttrDescriptions[1].binding = 0;
	vertexAttrDescriptions[1].location = 1;
	vertexAttrDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttrDescriptions[1].offset = offsetof(Vertex, normal);

	return vertexAttrDescriptions;
}

uint32_t Flux::Sphere::GetIndexCount()
{
	return numIndices;
}
