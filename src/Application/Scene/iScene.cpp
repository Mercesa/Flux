#include "iScene.h"

#include "Application/Camera.h"

using namespace Flux;

const std::shared_ptr<Camera> iScene::GetCamera() const
{
	return mCamera;
}

const std::vector<std::shared_ptr<iSceneObject>> Flux::iScene::GetSceneObjects() const
{
	return mSceneObjects;
}
