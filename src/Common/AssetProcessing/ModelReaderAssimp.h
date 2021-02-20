#pragma once
#include "Common/AssetProcessing/iModelReader.h"

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Flux
{
    class ModelReaderAssimp :
        public iModelReader
    {
    public:
        virtual bool CanRead(std::filesystem::path const& aFilepath) override;
        virtual std::shared_ptr<ModelAsset> LoadModel(std::filesystem::path const& aFilepath) override;

    private:
        static void ProcessNode(aiNode* const a_Node, const aiScene* const a_Scene, std::vector<std::shared_ptr<MeshAsset>>& aData);
        static std::shared_ptr<MeshAsset> ProcessMesh(aiMesh* const a_Mesh, const aiScene* const a_Scene);
    };
};


