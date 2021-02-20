#pragma once

#include "AssetObjects.h"
#include "iTextureReader.h"
#include "iModelReader.h"
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace Flux
{
	class ErrorUnsupportedAssetType : public std::exception
	{
		std::string mMessage;

	public:
		ErrorUnsupportedAssetType(std::filesystem::path const &aFilepath);
		virtual const char *what() const noexcept override;
	};

	class AssetManager
	{
		std::vector<std::shared_ptr<iTextureReader>> mTextureReaders;
		std::vector<std::shared_ptr<iModelReader>> mModelReaders;


		std::unordered_map<std::string, std::shared_ptr<TextureAsset>> mLoadedTextures;
		std::unordered_map<std::string, std::shared_ptr<ModelAsset>> mLoadedModels;

	public:
		AssetManager();

		std::shared_ptr<TextureAsset> LoadTexture(std::filesystem::path const &aFilepath);
		std::shared_ptr<ModelAsset> LoadModel(std::filesystem::path const& aFilepath);

	};
}
