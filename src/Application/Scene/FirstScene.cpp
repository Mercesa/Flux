#include "FirstScene.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include "Application/Camera.h"
#include "Application/Input.h"
#include "Common/AssetProcessing/ModelReaderAssimp.h"
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
