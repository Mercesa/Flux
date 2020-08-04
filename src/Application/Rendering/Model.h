#pragma once

#include <vector>
#include <memory>
#include "Application/Rendering/Mesh.h"

namespace Flux
{
class Model
{
public:
	std::vector<std::shared_ptr<Mesh>> mMeshes;
};
}
