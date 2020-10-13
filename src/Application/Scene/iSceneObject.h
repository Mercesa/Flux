#pragma once
#include "Common/AssetProcessing/AssetObjects.h"
#include "Application/Rendering/Mesh.h"
#include "Application/Rendering/Material.h"
#include <memory>

namespace Flux
{
class iSceneObject
{
public:
	iSceneObject() : mMesh(nullptr), mAsset(nullptr) {}
	virtual ~iSceneObject() = default;

	std::shared_ptr<Mesh> mMesh;
	std::shared_ptr<MeshAsset> mAsset;
	std::shared_ptr<Material> mMaterial;
};

}