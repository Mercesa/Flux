#pragma once
#include <memory>

#include "Renderer/BufferGPU.h"

namespace Flux
{
class MeshVK
{
public:
	MeshVK() = default;
	~MeshVK() = default;

	std::shared_ptr<Gfx::BufferGPU> mVertexBuffer = nullptr;
	std::shared_ptr<Gfx::BufferGPU> mIndexBuffer = nullptr;
};
}
