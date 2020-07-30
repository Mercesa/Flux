#include "CustomRenderer.h"

using namespace Flux;



Flux::CustomRenderer::CustomRenderer(std::shared_ptr<Camera> aCamera) : mCamera(aCamera), mRenderer(nullptr)
{
	mRenderer = std::make_unique<Renderer>(mCamera);
}

void CustomRenderer::Init()
{
	mRenderer->Init();
}

void CustomRenderer::Draw()
{
	mRenderer->Draw();
}

void CustomRenderer::Cleanup()
{
	
}
