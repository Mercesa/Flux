#pragma once
#include <memory>

#include "Renderer/BufferVK.h"

namespace Flux
{
class Mesh
{
public:
	Mesh() = default;
	~Mesh() = default;

	std::shared_ptr<BufferVK> mVertexBuffer = nullptr;
	std::shared_ptr<BufferVK> mIndexBuffer = nullptr;
};
}
