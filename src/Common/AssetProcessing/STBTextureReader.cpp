#include "STBTextureReader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

namespace Flux
{
	bool STBTextureReader::CanRead(std::filesystem::path const &aFilepath)
	{
		auto tExtension = aFilepath.extension();
		return tExtension == ".png" || tExtension == ".jpg" || tExtension == ".bmp" || tExtension == ".tga";
	}

	std::shared_ptr<TextureAsset> STBTextureReader::LoadTexture(std::filesystem::path const &aFilepath)
	{
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
		tTextureAsset->mData.resize(tDataSize);
		memcpy(tTextureAsset->mData.data(), tPixels, tDataSize);
		return tTextureAsset;
	}
}
