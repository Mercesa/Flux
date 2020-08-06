#pragma once
#include <memory>

#include "Application/Scene/iScene.h"

class Camera;
namespace Flux
{
class FirstScene : public iScene
{
public:
	FirstScene(std::shared_ptr<Input> aInput);
	virtual ~FirstScene();

	virtual void Init() final;
	virtual void Update(float aDt) final;
	virtual void Cleanup() final;

};
}

