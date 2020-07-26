#pragma once

#include "AssetObjects.h"

namespace Flux
{

class iTextureLoader
{
public:
	iTextureLoader() = default;
	virtual ~iTextureLoader() = default;

	virtual std::shared_ptr<TextureAsset> LoadModel(std::string aFilepath) = 0;
};

}