#pragma once

#include "vulkan/vulkan.h"

#include "Renderer/RootSignature.h"

#include <memory>

namespace Flux
{
	namespace Gfx
	{

		enum class FrontFace
		{
			eClockWise,
			eCounterClockWise
		};

		enum class CullModes
		{
			eCullNone,
			eCullFront,
			eCullBack,
			eCullFrontAndBack
		};

		struct RasterizerState
		{
			RasterizerState() : frontFaceMode(FrontFace::eCounterClockWise), cullMode(CullModes::eCullBack)
			{}

			Flux::Gfx::FrontFace frontFaceMode;
			Flux::Gfx::CullModes cullMode;
		};

		struct GraphicsPipelineCreateDesc
		{
			std::weak_ptr<RootSignature>						mRootSig;
			std::weak_ptr<RenderTarget>							mRt;
			Flux::Gfx::RasterizerState*							mRasterizer;
			VkVertexInputBindingDescription						bindingDescription{};
			std::vector<VkVertexInputAttributeDescription>		vertexAttrDescriptions;
			float												mPushConstantSize;
		};

		struct GraphicsPipeline
		{
			VkPipeline pipeline;
		};
	}
}