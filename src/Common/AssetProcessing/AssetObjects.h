#pragma once
#include <vector>
#include <memory>
#include <string>

#include <glm/gtx/common.hpp>

namespace Flux
{
	struct TextureAsset
	{
		TextureAsset() : mWidth(0), mHeight(0), mAmountOfChannels(0)
		{}

		std::string mFilePath; // might be better idea to use cpp17 filepath(?)
		std::vector<char> mData;
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mAmountOfChannels;
	};

	struct MaterialAsset
	{
		std::vector<std::pair<std::string, std::string>> mTextures;		// Type/Filepath
	};

	struct VertexData
	{
		VertexData() :
			position(glm::vec3(0.0f)),
			texCoords(glm::vec2(0.0f)),
			normal(glm::vec3(0.0f)),
			bitangent(glm::vec3(0.0f)),
			tangent(glm::vec3(0.0f))
		{}

		VertexData(glm::vec3 aPosition, glm::vec2 aTexCoords, glm::vec3 aNormal, glm::vec3 aBitangent, glm::vec3 aTangent) :
			position(aPosition),
			texCoords(aTexCoords),
			normal(aNormal),
			bitangent(aBitangent),
			tangent(aTangent)
		{}

		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
		glm::vec3 bitangent;
		glm::vec3 tangent;
	};

	// Might want to template the mesh asset class, so we can chose which precision we need for the indices
	struct MeshAsset
	{
		MeshAsset(std::vector<VertexData>& aVertexData, std::vector<uint32_t>& aIndices, MaterialAsset& aMaterialAsset) :
			mVertexData(aVertexData),
			mIndices(aIndices),
			mMaterialAsset(aMaterialAsset) {}
		MeshAsset() = default;

		std::vector<VertexData> mVertexData;
		std::vector<uint32_t> mIndices;
		MaterialAsset mMaterialAsset;
	};

	struct ModelAsset
	{
		ModelAsset(std::vector<std::shared_ptr<MeshAsset>>& aMeshes, std::string aFilePath) : mMeshes(aMeshes), mFilePath(aFilePath)
		{}

		std::vector<std::shared_ptr<MeshAsset>> mMeshes;
		std::string mFilePath;
	};
}