#include "STBTextureReader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <iostream>
namespace Flux
{
	bool STBTextureReader::CanRead(std::filesystem::path const &aFilepath)
	{
		auto tExtension = aFilepath.extension();
		return tExtension == ".png" || tExtension == ".jpg" || tExtension == ".bmp" || tExtension == ".tga";
	}

	std::shared_ptr<TextureAsset> STBTextureReader::LoadTexture(std::filesystem::path const &aFilepath)
	{
		stbi_set_flip_vertically_on_load(true);

		int tWidth, tHeight, tAmountOfChannels;
		stbi_uc const * const tPixels = stbi_load(aFilepath.string().c_str(),
												  &tWidth,
												  &tHeight,
												  &tAmountOfChannels,
												  STBI_rgb_alpha);

		if (!tPixels)
		{
			throw ErrorAssetFileNotFound(aFilepath);
		}

		std::shared_ptr<TextureAsset> const tTextureAsset = std::make_shared<TextureAsset>();
		tTextureAsset->mWidth = static_cast<uint32_t>(tWidth);
		tTextureAsset->mHeight = static_cast<uint32_t>(tHeight);
		uint32_t const tDataSize = tTextureAsset->mWidth * tTextureAsset->mHeight * 4;

		tTextureAsset->mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(tTextureAsset->mWidth, tTextureAsset->mHeight)))) + 1;

		tTextureAsset->mData.resize(tDataSize);
		tTextureAsset->mPath = aFilepath.string();
		memcpy(tTextureAsset->mData.data(), tPixels, tDataSize);
		stbi_image_free((void*)tPixels);

		return tTextureAsset;
	}
}
