#include "AssetManager.h"
#include "STBTextureReader.h"
#include "ModelReaderAssimp.h"

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
		// Add texture readers
		{
			mTextureReaders.push_back(std::make_shared<STBTextureReader>());
		}

		// Add model readers
		{
			mModelReaders.push_back(std::make_shared<ModelReaderAssimp>());
		}


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

	std::shared_ptr<ModelAsset> AssetManager::LoadModel(std::filesystem::path const& aFilepath)
	{
		if (mLoadedModels.find(aFilepath.string()) == mLoadedModels.end())
		{
			for (auto tModelReader : mModelReaders)
			{
				if (tModelReader->CanRead(aFilepath))
				{
					mLoadedModels[aFilepath.string()] = tModelReader->LoadModel(aFilepath);
					return mLoadedModels[aFilepath.string()];
				}
			}
			throw ErrorUnsupportedAssetType(aFilepath);
		}
		else
		{
			return mLoadedModels[aFilepath.string()];
		}

		return nullptr;
	}
}
