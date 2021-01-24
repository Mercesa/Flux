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
	Material() : mTextureAsset(nullptr), mTextureVK(nullptr), mIndex(matIDIndex++) {}
	std::shared_ptr<TextureAsset> mTextureAsset = nullptr;
	std::shared_ptr < Flux::Gfx::Texture > mTextureVK = nullptr;


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

