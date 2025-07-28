//
// Created by apant on 26/07/2025.
//

#ifndef VK_FENCE_H
#define VK_FENCE_H

#include <volk/volk.h>

namespace Gfx
{
void CreateFence(
	VkDevice               device,
	VkAllocationCallbacks* p_allocator,
	VkFence*               p_fence);
}

#endif //VK_FENCE_H
