#pragma once
#include "Common/AssetProcessing/iModelLoader.h"

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Flux
{
    class ModelReaderAssimp :
        public iModelLoader
    {
    public:
        virtual std::shared_ptr<ModelAsset> LoadModel(std::string aFilepath);

    private:
        static void ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene, std::vector<std::shared_ptr<MeshAsset>>& aData);
        static std::shared_ptr<MeshAsset> ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene);
    };
};


