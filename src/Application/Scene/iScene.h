#pragma once

#include <memory>
#include <vector>

#include "Application/Rendering/RenderDataStructs.h"

namespace Flux
{
	class Camera;
	class Input;
	class iSceneObject;
	class AssetManager;

	class iScene
	{
	public:
		iScene(std::shared_ptr<Camera> aCamera, std::shared_ptr<Input> aInput);
		virtual ~iScene() = default;

		virtual void Init() = 0;
		virtual void Update(float aDt) = 0;
		virtual void Cleanup() = 0;

		const std::shared_ptr<Camera> GetCamera() const;
		const std::vector<std::shared_ptr<iSceneObject>> GetSceneObjects() const;
		const std::vector<std::shared_ptr<Light>> GetLights() const;
		const double GetDelta();

	protected:
		std::shared_ptr<Camera> mCamera;
		std::shared_ptr<Input> mInput;
		std::vector<std::shared_ptr<iSceneObject>> mSceneObjects;
		std::shared_ptr<AssetManager> mAssetManager;
		std::vector<std::shared_ptr<Light>> mLights;
		double mDelta;

	};
}