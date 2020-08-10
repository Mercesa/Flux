#pragma once

#include "AssetObjects.h"

namespace Flux
{

class iTextureReader
{
public:
	iTextureReader() = default;
	virtual ~iTextureReader() = default;

	virtual bool CanRead(std::filesystem::path const &aFilepath) = 0;
	virtual std::shared_ptr<TextureAsset> LoadTexture(std::filesystem::path const &aFilepath) = 0;
};

}