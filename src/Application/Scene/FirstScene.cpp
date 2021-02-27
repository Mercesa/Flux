#include "FirstScene.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include "Application/Camera.h"
#include "Application/Input.h"
#include "Common/AssetProcessing/ModelReaderAssimp.h"
#include "Common/AssetProcessing/AssetManager.h"
#include "iSceneObject.h"
#include "Application/Rendering/RenderDataStructs.h"

#include "Application/Rendering/ImguiRenderingHelper.h"
#include "glm/gtc/type_ptr.hpp"

Flux::FirstScene::FirstScene(std::shared_ptr<Input> aInput) :
	iScene(
		std::make_shared<Camera>(glm::vec3(0.0f, 5.0f, 5.0f),
			glm::vec3(0.0f, 1.0f, 0.0f), YAW, -45.0f, 0.1f, 1000.0f, 90.0f, 1366, 768  ), aInput)
{
}

Flux::FirstScene::~FirstScene()
{
}

const std::string cPathCheckerboardTexture = std::string("Resources/Textures/checkerboard.jpg");

void Flux::FirstScene::Init()
{
	mCamera->MouseSensitivity = 1.0f;
	mCamera->MovementSpeed = 5.0f;


	auto tAsset = mAssetManager->LoadModel("Resources\\Models\\Sponza\\sponza.obj");

	for (auto& mesh : tAsset->mMeshes)
	{
		std::shared_ptr<iSceneObject> tSceneObject = std::make_shared<iSceneObject>();
		tSceneObject->mAsset = mesh;
		tSceneObject->mMaterial = std::make_shared<Material>();

		if (tSceneObject->mAsset->mMaterialAsset.mTextures.size() != 0)
		{
			for (auto& texture : tSceneObject->mAsset->mMaterialAsset.mTextures)
			{
				std::filesystem::path tPath = std::filesystem::path("Resources/Models/Sponza/" + texture.second);

				if (texture.first == "Diffuse")
				{
					tSceneObject->mMaterial->mTextureAssetAlbedo = mAssetManager->LoadTexture(tPath);
				}
				if (texture.first == "Specular")
				{
					tSceneObject->mMaterial->mTextureAssetSpecular = mAssetManager->LoadTexture(tPath);
				}
				if (texture.first == "Height")
				{
					tSceneObject->mMaterial->mTextureAssetNormal = mAssetManager->LoadTexture(tPath);
				}
			}
		}

		if(!tSceneObject->mMaterial->mTextureAssetAlbedo)
		{
			std::filesystem::path tPath = std::filesystem::path(cPathCheckerboardTexture);
			tSceneObject->mMaterial->mTextureAssetAlbedo = mAssetManager->LoadTexture(tPath);
		}

		if (!tSceneObject->mMaterial->mTextureAssetSpecular)
		{
			std::filesystem::path tPath = std::filesystem::path(cPathCheckerboardTexture);
			tSceneObject->mMaterial->mTextureAssetSpecular = mAssetManager->LoadTexture(tPath);
		}

		if (!tSceneObject->mMaterial->mTextureAssetNormal)
		{
			std::filesystem::path tPath = std::filesystem::path(cPathCheckerboardTexture);
			tSceneObject->mMaterial->mTextureAssetNormal = mAssetManager->LoadTexture(tPath);
		}

		tSceneObject->mRenderState.shaders.push_back({ ShaderTypes::eVertex, 		"Resources/Shaders/basicModel.vert.spv" });
		tSceneObject->mRenderState.shaders.push_back({ ShaderTypes::eFragment, 		"Resources/Shaders/basicModel.frag.spv" });

		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));

		tSceneObject->transform = scaleMatrix;

		this->mSceneObjects.push_back(tSceneObject);

	}

	auto tAssetDragon = mAssetManager->LoadModel("Resources\\Models\\Dragon\\dragon.obj");

	for (auto& mesh : tAssetDragon->mMeshes)
	{
		std::shared_ptr<iSceneObject> tSceneObject = std::make_shared<iSceneObject>();
		tSceneObject->mAsset = mesh;
		tSceneObject->mMaterial = std::make_shared<Material>();

		if (tSceneObject->mAsset->mMaterialAsset.mTextures.size() != 0)
		{
			for (auto& texture : tSceneObject->mAsset->mMaterialAsset.mTextures)
			{
				std::filesystem::path tPath = std::filesystem::path("Resources/Models/Dragon/" + texture.second);

				if (texture.first == "Diffuse")
				{
					tSceneObject->mMaterial->mTextureAssetAlbedo = mAssetManager->LoadTexture(tPath);
				}
				if (texture.first == "Specular")
				{
					tSceneObject->mMaterial->mTextureAssetSpecular = mAssetManager->LoadTexture(tPath);
				}
				if (texture.first == "Height")
				{
					tSceneObject->mMaterial->mTextureAssetNormal = mAssetManager->LoadTexture(tPath);
				}
			}
		}

		if (!tSceneObject->mMaterial->mTextureAssetAlbedo)
		{
			std::filesystem::path tPath = std::filesystem::path(cPathCheckerboardTexture);
			tSceneObject->mMaterial->mTextureAssetAlbedo = mAssetManager->LoadTexture(tPath);
		}

		if (!tSceneObject->mMaterial->mTextureAssetSpecular)
		{
			std::filesystem::path tPath = std::filesystem::path(cPathCheckerboardTexture);
			tSceneObject->mMaterial->mTextureAssetSpecular = mAssetManager->LoadTexture(tPath);
		}

		if (!tSceneObject->mMaterial->mTextureAssetNormal)
		{
			std::filesystem::path tPath = std::filesystem::path(cPathCheckerboardTexture);
			tSceneObject->mMaterial->mTextureAssetNormal = mAssetManager->LoadTexture(tPath);
		}

		tSceneObject->mRenderState.shaders.push_back({ ShaderTypes::eVertex, 		"Resources/Shaders/basicModel.vert.spv" });
		tSceneObject->mRenderState.shaders.push_back({ ShaderTypes::eFragment, 		"Resources/Shaders/basicModel.frag.spv" });

		glm::mat4 scaleMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		tSceneObject->transform = scaleMatrix;

		this->mSceneObjects.push_back(tSceneObject);

	}

	for (uint32_t y = 0; y < 3; ++y)
	{
		for (uint32_t x = 0; x < 3; ++x)
		{
			for (uint32_t z = 0; z < 3; ++z)
			{
				std::shared_ptr<Light> l = std::make_shared<Light>();
				l->constant = 1.0f;
				l->linear = 0.22f;
				l->quadratic = 0.2f;
				l->position = glm::vec3(x * 20, y * 5.0f, z * 5.0f);
				l->color = glm::vec3(10.0f, 10.0f, 10.0f );

				this->mLights.push_back(l);
			}
		}
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

	static float angle = 0.0f;
	angle += aDt * 5.0f;
	//mLights[0]->position = glm::vec3(0.0f, 6.0f, 0.0f) + glm::vec3(5 * sinf(angle), 0.0f, 0.0f);

	mCamera->ProcessMouseMovement(static_cast<float>(mInput->GetRelMousePos().x), static_cast<float>(mInput->GetRelMousePos().y), true);


}

void Flux::FirstScene::Cleanup()
{
}
