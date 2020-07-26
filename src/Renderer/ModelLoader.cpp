#include "ModelLoader.h"

#include <fstream>
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Exporter.hpp"

using namespace Flux;

bool is_file_exist(const char* fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

std::vector<VertexData> ProcessVertices(aiMesh* const a_Mesh)
{
	std::vector<VertexData> tVertexData;
	// Loop through vertices
	for (unsigned int i = 0; i < a_Mesh->mNumVertices; i++)
	{
		aiVector3D& vertice = a_Mesh->mVertices[i];
		aiVector3D& tang = a_Mesh->mTangents[i];
		aiVector3D& bTang = a_Mesh->mBitangents[i];

		VertexData vert;

		// Set position
		vert.position.x = vertice.x;
		vert.position.y = vertice.y;
		vert.position.z = vertice.z;

		if (a_Mesh->HasTangentsAndBitangents())
		{
			// Set Tang
			vert.tangent.x = tang.x;
			vert.tangent.y = tang.y;
			vert.tangent.z = tang.z;

			// Set BiTang
			vert.bitangent.x = bTang.x;
			vert.bitangent.y = bTang.y;
			vert.bitangent.z = bTang.z;
		}

		// Set normals
		if (a_Mesh->HasNormals())
		{
			vert.normal.x = a_Mesh->mNormals[i].x;
			vert.normal.y = a_Mesh->mNormals[i].y;
			vert.normal.z = a_Mesh->mNormals[i].z;
		}

		// Set tex coords
		if (a_Mesh->HasTextureCoords(0))
		{
			//glm::vec2 tex;
			vert.texCoords.x = a_Mesh->mTextureCoords[0][i].x;
			vert.texCoords.y = a_Mesh->mTextureCoords[0][i].y;
		}

		// Put it in vec
		tVertexData.push_back(vert);
	}
	return tVertexData;
}


// Process indices
std::vector<uint32_t> ProcessIndices(aiMesh* const a_Mesh)
{
	std::vector<uint32_t> tIndices;
	if (a_Mesh->HasFaces())
	{
		for (unsigned int i = 0; i < a_Mesh->mNumFaces; i++)
		{
			for (unsigned int j = 0; j < a_Mesh->mFaces[i].mNumIndices; j++)
			{
				tIndices.push_back(a_Mesh->mFaces[i].mIndices[j]);
			}
		}
	}

	return tIndices;
}

bool DoesMaterialHaveTextures(aiMaterial* const aMat, aiTextureType a_Type)
{
	// Check the amount of textures of a specific type
	if ((aMat->GetTextureCount(a_Type)) <= 0)
	{
		return false;
	}

	return true;
}

std::string GetTextureLocation(aiMaterial* const a_Mat, aiTextureType a_Type)
{
	aiString str;
	a_Mat->GetTexture(a_Type, 0, &str);

	std::string stString = std::string(str.C_Str());


	// Add DDS to the file extension
	//std::size_t dotPos = stString.find_last_of(".");
	//stString.erase(dotPos, stString.size());
	//
	//stString.append(".DDS");

	return stString;
}


// Goes through the material
MaterialAsset ProcessMaterial(aiMesh* a_Mesh, const aiScene* a_Scene)
{
	// If we have a material
	if (a_Mesh->mMaterialIndex > 0)
	{
		MaterialAsset tMaterialAsset;
		aiMaterial* const material = a_Scene->mMaterials[a_Mesh->mMaterialIndex];


		if (DoesMaterialHaveTextures(material, aiTextureType_DIFFUSE))
		{
			tMaterialAsset.mTextures.push_back(std::pair<std::string, std::string>("Diffuse", GetTextureLocation(material, aiTextureType_DIFFUSE)));
		}

		if (DoesMaterialHaveTextures(material, aiTextureType_SPECULAR))
		{
			tMaterialAsset.mTextures.push_back(std::pair<std::string, std::string>("Specular", GetTextureLocation(material, aiTextureType_SPECULAR)));
		}

		if (DoesMaterialHaveTextures(material, aiTextureType_HEIGHT))
		{
			tMaterialAsset.mTextures.push_back(std::pair<std::string, std::string>("Height", GetTextureLocation(material, aiTextureType_HEIGHT)));

		}
		return tMaterialAsset;
	}

	return MaterialAsset();
}


// Process mesh
std::shared_ptr<MeshAsset> ModelLoader::ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene)
{
	std::vector<VertexData> vertices = std::move(ProcessVertices(a_Mesh));
	std::vector<uint32_t> indices = std::move(ProcessIndices(a_Mesh));
	Flux::MaterialAsset material = ProcessMaterial(a_Mesh, a_Scene);
	// Process vertices & indices


	std::shared_ptr<MeshAsset> meshData = std::make_shared<MeshAsset>(
		vertices,
		indices,
		material);


	return meshData;
}

// This function will be called recursively if there is more than 1 node in a scene
void ModelLoader::ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene, std::vector<std::shared_ptr<MeshAsset>>& aData)
{
	// Process meshes in node
	for (unsigned int i = 0; i < a_Node->mNumMeshes; i++)
	{
		aData.push_back(ProcessMesh(a_Scene->mMeshes[a_Node->mMeshes[i]], a_Scene));
	}

	// Process children of scene recursively
	for (unsigned int j = 0; j < a_Node->mNumChildren; j++)
	{
		ProcessNode((a_Node->mChildren[j]), a_Scene, aData);
	}
}

std::shared_ptr<ModelAsset> Flux::ModelLoader::LoadModel(std::string aFilepath)
{
	std::shared_ptr<ModelAsset> tModelAsset;

	bool doesAssbinVersionExist = false;
	// Set directory string and c_string

	const aiScene* scene;
	Assimp::Importer importer;

	scene = importer.ReadFile(aFilepath.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_CalcTangentSpace | aiProcess_GenUVCoords);

	assert(scene != nullptr);
	if (!scene)
	{
		printf("%s", importer.GetErrorString());
		return std::shared_ptr<ModelAsset>();
	}


	std::vector<std::shared_ptr<MeshAsset>> tData;
	ProcessNode(scene->mRootNode, scene, tData);


	return std::make_shared<ModelAsset>(tData, aFilepath);
}
