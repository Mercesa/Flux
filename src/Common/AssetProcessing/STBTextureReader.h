#pragma once

#include "iTextureReader.h"

namespace Flux
{
	class STBTextureReader : public iTextureReader
	{
	public:
		virtual bool CanRead(std::filesystem::path const &aFilepath) override;
		virtual std::shared_ptr<TextureAsset> LoadTexture(std::filesystem::path const &aFilepath) override;
	};
}
