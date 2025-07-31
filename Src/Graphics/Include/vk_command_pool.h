//
// Created by apant on 26/07/2025.
//

#ifndef VK_COMMAND_POOL_H
#define VK_COMMAND_POOL_H

#include <volk/volk.h>

namespace Gfx
{
void CreateCommandPool(
	VkDevice                 device,
	VkCommandPoolCreateFlags flags,
	uint32_t                 queue_family_idx,
	VkAllocationCallbacks*   p_allocator,
	VkCommandPool*           p_command_pool);

void DestroyCommandPool(
	VkDevice               device,
	VkCommandPool          command_pool,
	VkAllocationCallbacks* p_allocator);
}

#endif //VK_COMMAND_POOL_H
