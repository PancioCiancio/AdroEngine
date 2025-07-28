//
// Created by apant on 26/07/2025.
//

#include "vk_command_pool.h"
#include "vk_utils.h"

namespace Gfx
{
void CreateCommandPool(
	VkDevice                 device,
	VkCommandPoolCreateFlags flags,
	uint32_t                 queue_family_idx,
	VkAllocationCallbacks*   p_allocator,
	VkCommandPool*           p_command_pool)
{
	const VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = flags,
		.queueFamilyIndex = queue_family_idx,
	};

	VK_CHECK(vkCreateCommandPool(
		device,
		&command_pool_create_info,
		p_allocator,
		p_command_pool));
}
}
