#pragma once
#include <memory>
#include <vector>
#include <string>

#include <Common/AssetProcessing/AssetObjects.h>
#include <Renderer/TextureVK.h>

namespace Flux
{
class Material
{
public:
	Material() : mTextureAssetAlbedo(nullptr), mTextureAlbedo(nullptr), mIndex(matIDIndex++) {}

	std::shared_ptr<TextureAsset> mTextureAssetAlbedo = nullptr;
	std::shared_ptr<TextureAsset> mTextureAssetSpecular = nullptr;
	std::shared_ptr<TextureAsset> mTextureAssetNormal = nullptr;

	std::shared_ptr < Flux::Gfx::Texture > mTextureAlbedo = nullptr;
	std::shared_ptr < Flux::Gfx::Texture > mTextureSpecular = nullptr;
	std::shared_ptr < Flux::Gfx::Texture > mTextureNormal = nullptr;


	VkDescriptorSet mDescriptorSet;
	uint64_t mIndex;
	static uint64_t matIDIndex;

	//void operator = (const Flux::Material& M) {
	//	this->matIDIndex = M.matIDIndex;
	//	this->mTextureAsset = M.mTextureAsset;
	//	this->mDescriptorSet = M.mDescriptorSet;
	//	this->mTextureVK = M.mTextureVK;
	//}

};
}

