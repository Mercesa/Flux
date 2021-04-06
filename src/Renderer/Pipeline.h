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

		enum class DepthCompareOp
		{
			eCompareNever,
			eCompareLess,
			eCompareEqual,
			eCompareLessOrEqual,
			eCompareGreater,
			eCompareNotEqual,
			eCompareGreaterOrEqual,
			eCompareAlways
		};

		struct RasterizerState
		{
			RasterizerState() : frontFaceMode(FrontFace::eCounterClockWise), cullMode(CullModes::eCullBack)
			{}

			Flux::Gfx::FrontFace frontFaceMode;
			Flux::Gfx::CullModes cullMode;
		};

		struct DepthStencilState
		{
			DepthStencilState() : depthTestEnable(true), depthWriteEnable(true), depthCompareOp(DepthCompareOp::eCompareLess)
			{}

			bool depthTestEnable;
			bool depthWriteEnable;
			Flux::Gfx::DepthCompareOp depthCompareOp;

		};

		struct GraphicsPipelineCreateDesc
		{
			std::weak_ptr<RootSignature>						mRootSig;
			std::weak_ptr<RenderTarget>							mRt;
			Flux::Gfx::RasterizerState							mRasterizer;
			Flux::Gfx::DepthStencilState						mDepthStencilState;
			VkVertexInputBindingDescription						bindingDescription{};
			std::vector<VkVertexInputAttributeDescription>		vertexAttrDescriptions;
			float												mPushConstantSize;
		};

		struct GraphicsPipeline
		{
			VkPipeline pipeline;
			std::weak_ptr<RootSignature> mRootSignature;
		};

		struct ComputePipelineCreatedesc
		{
			std::weak_ptr<RootSignature>						mRootSig;
		};

		struct ComputePipeline
		{
			VkPipeline computePipeline;
			std::weak_ptr<RootSignature> mRootSignature;
		};
	}
}