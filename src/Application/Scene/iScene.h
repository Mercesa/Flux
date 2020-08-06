#pragma once

#include <memory>
namespace Flux
{
	class Camera;
	class Input;
	class iScene
	{
	public:
		iScene(std::shared_ptr<Camera> aCamera, std::shared_ptr<Input> aInput) : mCamera(aCamera), mInput(aInput)
		{}
		virtual ~iScene() = default;

		virtual void Init() = 0;
		virtual void Update(float aDt) = 0;
		virtual void Cleanup() = 0;

		const std::shared_ptr<Camera> GetCamera();

	protected:
		std::shared_ptr<Camera> mCamera;
		std::shared_ptr<Input> mInput;
	};
}