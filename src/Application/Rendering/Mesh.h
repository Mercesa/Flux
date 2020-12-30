#pragma once
#include <memory>

#include "Renderer/BufferVK.h"

namespace Flux
{
class MeshVK
{
public:
	MeshVK() = default;
	~MeshVK() = default;

	std::shared_ptr<BufferVK> mVertexBuffer = nullptr;
	std::shared_ptr<BufferVK> mIndexBuffer = nullptr;
};
}
