#pragma once


namespace Flux
{
namespace Gfx
{
	struct DescriptorPoolCreateDesc
	{
		uint32_t maxDescriptorSets;
	};

	struct DescriptorPool
	{
		VkDescriptorPool mPool;
	};
}
}