#pragma once

#include "AssetObjects.h"

namespace Flux
{
	// Loads models
	// Model contains meshes
	// Meshes contain geometry data and indices/vertices
	class iModelLoader
	{
	public:
		iModelLoader() = default;
		virtual ~iModelLoader() = default;

		virtual std::shared_ptr<ModelAsset> LoadModel(std::string aFilepath) = 0;

	};
}