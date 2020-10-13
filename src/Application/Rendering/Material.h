#pragma once
#include <memory>
#include <Common/AssetProcessing/AssetObjects.h>
#include <Renderer/TextureVK.h>

namespace Flux
{
class Material
{
public:
	Material() : mTextureAsset(nullptr), mTextureVK(nullptr) {}
	std::shared_ptr<TextureAsset> mTextureAsset = nullptr;
	std::shared_ptr<TextureVK> mTextureVK = nullptr;

	VkDescriptorSet mDescriptorSet;
};
}

