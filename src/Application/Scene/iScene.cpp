#include "iScene.h"

#include "Application/Camera.h"
#include "Common/AssetProcessing/AssetManager.h"
using namespace Flux;

iScene::iScene(std::shared_ptr<Camera> aCamera, std::shared_ptr<Input> aInput) : mCamera(aCamera), mInput(aInput), mAssetManager(std::make_shared<AssetManager>())
{}

const std::shared_ptr<Camera> iScene::GetCamera() const
{
	return mCamera;
}

const std::vector<std::shared_ptr<iSceneObject>> Flux::iScene::GetSceneObjects() const
{
	return mSceneObjects;
}
