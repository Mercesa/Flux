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

	std::shared_ptr<BufferVK> mVertexBuffer;
	std::shared_ptr<BufferVK> mIndexBuffer;
};
}
