#include "pch.h"

#include "Common/FileHandling/FileReadUtility.h"
#include "Renderer/ShaderReflection.h"

using namespace Flux::Gfx::ShaderReflection;

TEST(ShaderReflectionTest, fragment) {
	std::vector<char> code;
	ASSERT_NO_THROW(code = Flux::Common::ReadFile<char>("Resources/Shaders/basicModel.frag.spv"));

	ShaderReflection reflection{};
	ASSERT_NO_THROW(reflection = Reflect(code));

	std::vector<ShaderResourceReflection> images;

	for (const auto& e : reflection.mResources)
	{
		if (e.mType == ShaderResourceType::eSeparateImages)
		{
			images.push_back(e);
		}
	}

	ASSERT_EQ(images.size(), 3);

	const auto& albedo		= std::find_if(images.begin(), images.end(), [](const ShaderResourceReflection x) {return x.mName == "textureAlbedo"; });
	const auto& specular	= std::find_if(images.begin(), images.end(), [](const ShaderResourceReflection x) {return x.mName == "textureSpecular"; });
	const auto& normal		= std::find_if(images.begin(), images.end(), [](const ShaderResourceReflection x) {return x.mName == "textureNormal"; });

	EXPECT_NE(normal,	images.end());
	EXPECT_NE(specular, images.end());
	EXPECT_NE(normal,	images.end());
}
