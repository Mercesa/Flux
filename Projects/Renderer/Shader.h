#pragma once

namespace Flux
{
	namespace Gfx
	{
		enum class ShaderTypes
		{
			eVertex,
			eFragment,
			eGeometry,
			eTessellationControl,
			eTessellationEval,
			eCompute,
			eRayGen,
			eRayClosestHit,
			eRayMiss,
			eRayAnyHit,
			eRayIntersection,
			eRayCallable
		};
	}
}