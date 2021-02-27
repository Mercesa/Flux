#pragma once

#include "vulkan/vulkan.h"

namespace Flux
{
	namespace Gfx
	{
		enum eQueueType
		{
			QUEUE_TYPE_GRAPHICS = 0,
			QUEUE_TYPE_PRESENT = 1,
			QUEUE_TYPE_TRANSFER = 2,
			QUEUE_TYPE_COMPUTE = 3,
			MAX_QUEUE_TYPE
		};

		struct QueueCreateDesc
		{
			eQueueType mType;
		};

		struct Queue {
			eQueueType mType;


			// Vulkan
			uint32_t mQueueIndex;
			VkQueue mVkQueue;
		};
	}
}