#pragma once

#include "vulkan/vulkan.h"

namespace Flux
{
	namespace Gfx
	{
		enum eQueueType
		{
			QUEUE_TYPE_GRAPHICS = 1,
			QUEUE_TYPE_PRESENT = 1 << 1,
			QUEUE_TYPE_TRANSFER = 1 << 2,
			QUEUE_TYPE_COMPUTE = 1 << 3,
			MAX_QUEUE_TYPE
		};

		struct QueueCreateDesc
		{
			eQueueType mType;
		};

		struct Queue {
			eQueueType mType;

			uint32_t mQueueIndex;
			VkQueue mVkQueue;
		};
	}
}