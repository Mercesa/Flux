#pragma once


namespace Flux
{
	namespace Gfx
	{
		enum ShaderTypes
		{
			eVertex = 1 << 0,
			eFragment = 1 << 1,
			eGeometry = 1 << 2,
			eTessellationControl = 1 << 3,
			eTessellationEval = 1 << 4,
			eCompute = 1 << 5,
			eRayGen = 1 << 6,
			eRayClosestHit = 1 << 7,
			eRayMiss = 1 << 8,
			eRayAnyHit = 1 << 9,
			eRayIntersection = 1 << 10,
			eRayCallable = 1 << 11,
			eUnknownShaderType = 0
		};
	}
}