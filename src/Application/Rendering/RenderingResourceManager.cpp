#include "RenderingResourceManager.h"



Flux::RenderingResourceManager::RenderingResourceManager()
{
}

// Check for empty pointers, check if resource is already registered, if already registered. return true
std::optional<std::shared_ptr<Flux::TextureVK>> Flux::RenderingResourceManager::QueryTextureAssetRegistered(std::shared_ptr<Flux::TextureAsset> aTextureAsset) const
{
	assert(aTextureAsset);

	for (auto& texture : mTextures)
	{
		if (aTextureAsset->mPath == texture.first->mPath)
		{
			return std::optional<std::shared_ptr<Flux::TextureVK>>(texture.second);
		}
	}

	// Asset is not associated with a VK texture yet
	return std::optional<std::shared_ptr<Flux::TextureVK>>();
}

bool Flux::RenderingResourceManager::RegisterTextureData(std::pair < std::shared_ptr<Flux::TextureAsset>, std::shared_ptr<Flux::TextureVK>> aTexturePair)
{
	assert(aTexturePair.first);
	assert(aTexturePair.second);

	// If the asset exists, then we can't register it again
	if (QueryTextureAssetRegistered(aTexturePair.first))
	{
		return false;
	}

	mTextures.push_back(aTexturePair);

	return true;
}

std::optional<std::shared_ptr<Flux::Material>> Flux::RenderingResourceManager::QueryMaterialAssetRegistered(std::shared_ptr<Flux::Material> aMaterial) const
{
	assert(aMaterial);

	// Material needs to be associated with a textureVK struct or a texture asset
	if (!aMaterial->mTextureVK || !aMaterial->mTextureAsset)
	{
		return std::optional<std::shared_ptr<Flux::Material>>();
	}

	// If asset is not registered yet, return nothing
	if (!QueryTextureAssetRegistered(aMaterial->mTextureAsset))
	{
		return std::optional<std::shared_ptr<Flux::Material>>();
	}

	for (auto& material : mMaterials)
	{
		// TODO: just now check the path, in the future check several paths
		if ((aMaterial->mTextureAsset->mPath == material->mTextureAsset->mPath))
		{
			return std::optional<std::shared_ptr<Flux::Material>>(material);
		}
	}

	// Does not exist
	return std::optional<std::shared_ptr<Flux::Material>>();
}

bool Flux::RenderingResourceManager::RegisterMaterial(std::shared_ptr<Flux::Material> aMaterial)
{
	assert(aMaterial);

	// If the asset exists, then we can't register it again
	if (QueryMaterialAssetRegistered(aMaterial))
	{
		return false;
	}

	mMaterials.push_back(aMaterial);

	return true;
}

void Flux::RenderingResourceManager::Update()
{
}
