#include "AssetManager.h"
#include "STBTextureReader.h"

namespace Flux
{
	ErrorUnsupportedAssetType::ErrorUnsupportedAssetType(std::filesystem::path const &aFilepath)
	{
		mMessage = "Failed to load '" + aFilepath.string() + "'. File extension '" + aFilepath.extension().string() + "' not supported.";
	}

	const char *ErrorUnsupportedAssetType::what() const noexcept
	{
		return mMessage.c_str();
	}

	AssetManager::AssetManager()
	{
		mTextureReaders.push_back(std::make_shared<STBTextureReader>());
	}

	std::shared_ptr<TextureAsset> AssetManager::LoadTexture(std::filesystem::path const &aFilepath)
	{
		// Find a texture, if its found, return existing texture. Otherwise load it in.
		if (mLoadedTextures.find(aFilepath.string()) == mLoadedTextures.end())
		{
			for (auto tTextureReader : mTextureReaders)
			{
				if (tTextureReader->CanRead(aFilepath))
				{
					mLoadedTextures[aFilepath.string()] = tTextureReader->LoadTexture(aFilepath);
					return mLoadedTextures[aFilepath.string()];
				}
			}
			throw ErrorUnsupportedAssetType(aFilepath);
		}
		else
		{
			return mLoadedTextures[aFilepath.string()];
		}
		return nullptr;
	}
}
