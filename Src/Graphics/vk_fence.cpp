//
// Created by apant on 26/07/2025.
//

#include "vk_fence.h"
#include "vk_utils.h"

namespace Gfx
{
void CreateFence(
	VkDevice               device,
	VkAllocationCallbacks* p_allocator,
	VkFence*               p_fence)
{
	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	VK_CHECK(vkCreateFence(
		device,
		&fence_info,
		p_allocator,
		p_fence));
}

}