#include "pch.h"

#include "Common/FileHandling/FileReadUtility.h"
#include "Renderer/ShaderReflection.h"
#include "Renderer/Shader.h"

using namespace Flux::Gfx::ShaderReflection;

TEST(ShaderReflectionTest, fragment) {
	std::vector<char> code;
	ASSERT_NO_THROW(code = Flux::Common::ReadFile<char>("Resources/Shaders/basicModel.frag.spv"));

	ShaderReflectionData reflection{};
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

TEST(ShaderReflectionTest, ValidationAndRetrieval) {
	std::vector<char> codeFragment;
	ASSERT_NO_THROW(codeFragment = Flux::Common::ReadFile<char>("Resources/Shaders/basicModel.frag.spv"));
	std::vector<char> codeVertex;
	ASSERT_NO_THROW(codeVertex = Flux::Common::ReadFile<char>("Resources/Shaders/basicModel.vert.spv"));

	std::shared_ptr<Flux::Gfx::Shader> fragmentShader = std::make_shared<Flux::Gfx::Shader>();
	std::shared_ptr<Flux::Gfx::Shader> vertexShader = std::make_shared<Flux::Gfx::Shader>();

	ASSERT_NO_THROW(fragmentShader->mReflectionData = Reflect(codeFragment));
	ASSERT_NO_THROW(vertexShader->mReflectionData = Reflect(codeVertex));

	const std::vector<std::shared_ptr<Flux::Gfx::Shader>> mShaders = { fragmentShader, vertexShader };
	auto shaderResourcesMerged = ValidateAndMergeShaderResources(mShaders);

	ASSERT_NE(shaderResourcesMerged.size(), 0);

	for (auto& shaderResource : shaderResourcesMerged)
	{
		std::cout << shaderResource.mName << " Set " << shaderResource.mSetNumber << " Binding " << shaderResource.mBindingNumber << std::endl;
	}

}