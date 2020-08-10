#include "AssetObjects.h"

namespace Flux
{
	ErrorAssetFileNotFound::ErrorAssetFileNotFound(std::filesystem::path const &aFilepath)
	{
		mMessage = "Failed to open file '" + aFilepath.string() + "'.";
	}

	const char *ErrorAssetFileNotFound::what() const noexcept
	{
		return mMessage.c_str();
	}
}
