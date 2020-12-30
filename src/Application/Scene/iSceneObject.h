#pragma once
#include <memory>

#include "Common/AssetProcessing/AssetObjects.h"
#include "Application/Rendering/Mesh.h"
#include "Application/Rendering/Material.h"
#include "Application/Rendering/RenderState.h"


namespace Flux
{
class iSceneObject
{
public:
	iSceneObject() : mMesh(nullptr), mAsset(nullptr) {}
	virtual ~iSceneObject() = default;

	std::shared_ptr<MeshVK> mMesh;
	std::shared_ptr<MeshAsset> mAsset;
	std::shared_ptr<Material> mMaterial;
	RenderState mRenderState;
	glm::mat4 transform;
};

}