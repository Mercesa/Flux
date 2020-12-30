#include "FirstScene.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include "Application/Camera.h"
#include "Application/Input.h"
#include "Common/AssetProcessing/ModelReaderAssimp.h"
#include "Common/AssetProcessing/AssetManager.h"
#include "iSceneObject.h"

Flux::FirstScene::FirstScene(std::shared_ptr<Input> aInput) :
	iScene(
		std::make_shared<Camera>(glm::vec3(0.0f, 5.0f, 5.0f),
			glm::vec3(0.0f, 1.0f, 0.0f), YAW, -45.0f), aInput)
{
}

Flux::FirstScene::~FirstScene()
{
}

void Flux::FirstScene::Init()
{
	mCamera->MouseSensitivity = 1.0f;
	mCamera->MovementSpeed = 5.0f;

	std::shared_ptr<ModelReaderAssimp> tLoader = std::make_shared<ModelReaderAssimp>();

	auto tAsset = tLoader->LoadModel("Resources\\Models\\Sponza\\sponza.obj");

	for (auto& mesh : tAsset->mMeshes)
	{
		std::shared_ptr<iSceneObject> tSceneObject = std::make_shared<iSceneObject>();
		tSceneObject->mAsset = mesh;
		tSceneObject->mMaterial = std::make_shared<Material>();

		if (tSceneObject->mAsset->mMaterialAsset.mTextures.size() != 0)
		{
			std::filesystem::path tPath = std::filesystem::path("Resources/Models/Sponza/" + tSceneObject->mAsset->mMaterialAsset.mTextures[0].second);
			tSceneObject->mMaterial->mTextureAsset = mAssetManager->LoadTexture(tPath);
		}

		if(!tSceneObject->mMaterial->mTextureAsset)
		{
			std::filesystem::path tPath = std::filesystem::path("Resources/Textures/checkerboard.jpg");
			tSceneObject->mMaterial->mTextureAsset = mAssetManager->LoadTexture(tPath);
		}

		tSceneObject->mRenderState.shaders.push_back({ ShaderTypes::eVertex, 		"Resources/Shaders/basicModel.vert.spv" });
		tSceneObject->mRenderState.shaders.push_back({ ShaderTypes::eFragment, 		"Resources/Shaders/basicModel.frag.spv" });

		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
		tSceneObject->transform = scaleMatrix;

		this->mSceneObjects.push_back(tSceneObject);

	}
}

void Flux::FirstScene::Update(float aDt)
{
	if (mInput->GetKeyHeld(GLFW_KEY_W))
	{
		mCamera->ProcessKeyboard(Flux::Camera_Movement::FORWARD, aDt);
	}
	if (mInput->GetKeyHeld(GLFW_KEY_A))
	{
		mCamera->ProcessKeyboard(Flux::Camera_Movement::LEFT, aDt);
	}
	if (mInput->GetKeyHeld(GLFW_KEY_S))
	{
		mCamera->ProcessKeyboard(Flux::Camera_Movement::BACKWARD, aDt);
	}
	if (mInput->GetKeyHeld(GLFW_KEY_D))
	{
		mCamera->ProcessKeyboard(Flux::Camera_Movement::RIGHT, aDt);
	}

	mCamera->ProcessMouseMovement(static_cast<float>(mInput->GetRelMousePos().x), static_cast<float>(mInput->GetRelMousePos().y), true);
}

void Flux::FirstScene::Cleanup()
{
}
