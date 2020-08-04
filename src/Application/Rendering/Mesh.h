#pragma once
#include <vulkan/vulkan.h>
#include <VmaUsage.h>

namespace Flux
{
class Mesh
{
public:
	Mesh() = default;
	~Mesh() = default;

	VkBuffer vertexBuffer;
	VmaAllocation vertexBufferMemory;
	VkBuffer indexBuffer;
	VmaAllocation indexBufferMemory;
};
}
