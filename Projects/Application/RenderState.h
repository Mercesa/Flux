#pragma once

#include <vector>

namespace Flux
{
enum ShaderTypes
{
	eVertex,
	eFragment
};

enum faceSides
{
	eFaceFront,
	eFaceBack
};

enum cullModes
{
	eCullNone,
	eCullFront,
	eCullBack,
	eCullFrontAndBack
};

struct RasterizerState
{
	faceSides frontFaceMode;
	cullModes cullMode;
};

class RenderState
{
	std::vector<ShaderTypes> shaders;
	RasterizerState drawState;
};

}