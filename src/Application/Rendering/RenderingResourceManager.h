#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <optional>

#include <Renderer/TextureVK.h>
#include "Common/AssetProcessing/AssetObjects.h"
#include "Application/Rendering/Material.h"

namespace Flux
{
	class TextureAsset;

class RenderingResourceManager
{
public:
	RenderingResourceManager();
	~RenderingResourceManager() = default;
	// Holds rendering resources
	// Responsibly for releasing resources at the right times, for example when they are not used
	// Not copyable, multiple instances can exist
	// Will keep track of how many times the resources are referenced, if not referenced at all anymore, feel free to delete (for now)
	// Will relate a resource asset to a vulkan asset

	// Bool: Returns if texture can be registered or not
	std::optional<std::shared_ptr<Flux::TextureVK>> QueryTextureAssetRegistered(std::shared_ptr<Flux::TextureAsset> aTextureAsset) const;
	bool RegisterTextureData(std::pair < std::shared_ptr<Flux::TextureAsset>, std::shared_ptr<Flux::TextureVK>> aTexturePair);
	// Update will check the resources currently there and how many references they have, if no references. Feel free to delete.

	std::optional<std::shared_ptr<Flux::Material>> QueryMaterialAssetRegistered(std::shared_ptr<Flux::Material> aMeshAsset) const;
	bool RegisterMaterial(std::shared_ptr<Flux::Material> aMeshAsset);

	void Update();

private:
	RenderingResourceManager(const RenderingResourceManager&) = delete;
	RenderingResourceManager& operator= (const RenderingResourceManager&) = delete;

	std::vector<std::pair<std::shared_ptr<Flux::TextureAsset>, std::shared_ptr<Flux::TextureVK>>> mTextures;
	std::vector<std::shared_ptr<Flux::Material>> mMaterials;
	std::vector<VkDescriptorSet> mSets;
};

}