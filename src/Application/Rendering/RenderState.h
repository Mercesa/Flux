#pragma once

#include <vector>
#include <string>
#include <optional>

namespace Flux
{
enum class ShaderTypes
{
	eVertex,
	eFragment
};

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

	FrontFace frontFaceMode;
	CullModes cullMode;
};

struct RenderState
{
	std::vector<std::pair<ShaderTypes, std::string>> shaders;
	RasterizerState drawState;
	std::optional<uint32_t> stateID;


	inline bool operator==(const RenderState& rhs) {

		if (this->shaders.size() != rhs.shaders.size())
		{
			return false;
		}

		if (drawState.cullMode != rhs.drawState.cullMode)
		{
			return false;
		}

		if (drawState.frontFaceMode != rhs.drawState.frontFaceMode)
		{
			return false;
		}

		for (auto& shaderlhs : shaders)
		{
			bool foundMatch = false;
			// Go through both shaders, try to find match, if no match found. The render states are fundamentally different.
			for (auto& shaderrhs : rhs.shaders)
			{
				if (shaderlhs.first == shaderrhs.first)
				{
					if (shaderlhs.second == shaderrhs.second)
					{
						foundMatch = true;
					}
				}
			}

			if (!foundMatch)
			{
				return false;
			}
		}

		return true;
	}

};

}