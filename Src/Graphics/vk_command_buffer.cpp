//
// Created by apant on 26/07/2025.
//

#include "vk_command_buffer.h"
#include "vk_utils.h"


namespace Gfx
{
void CreateCommandBuffer(
	VkDevice         device,
	VkCommandPool    command_pool,
	VkCommandBuffer* p_command_buffer)
{
	const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VK_CHECK(vkAllocateCommandBuffers(
		device,
		&command_buffer_allocate_info,
		p_command_buffer));
}
}
