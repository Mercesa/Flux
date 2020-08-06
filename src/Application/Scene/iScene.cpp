#include "iScene.h"

#include "Application/Camera.h"

using namespace Flux;

const std::shared_ptr<Camera> iScene::GetCamera()
{
	return mCamera;
}