#pragma once

#include "Renderer/Renderer.h"
#include "Application/Camera.h"

namespace Flux
{


class CustomRenderer
{
public:
	CustomRenderer(std::shared_ptr<Camera> aCamera);
	~CustomRenderer() = default;

	void Init();
	void Draw();
	void Cleanup();
private:
	std::shared_ptr<Camera> mCamera;
protected:
	std::unique_ptr<Renderer> mRenderer;
	
};

}