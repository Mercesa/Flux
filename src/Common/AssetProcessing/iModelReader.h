#pragma once

#include "AssetObjects.h"

namespace Flux
{
	class iModelReader
	{
	public:
		iModelReader() = default;
		virtual ~iModelReader() = default;

		virtual bool CanRead(std::filesystem::path const& aFilepath) = 0;
		virtual std::shared_ptr<ModelAsset> LoadModel(std::filesystem::path const& aFilepath) = 0;
	};
}