//
// Created by apant on 26/07/2025.
//

#include "vk_semaphore.h"
#include "vk_utils.h"

namespace Gfx
{
void CreateSemaphore(
	VkDevice               device,
	VkAllocationCallbacks* p_allocator,
	VkSemaphore*           p_semaphore)
{
	VkSemaphoreCreateInfo image_available_semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
	};

	VK_CHECK(vkCreateSemaphore(
		device,
		&image_available_semaphore_info,
		p_allocator,
		p_semaphore));
}
}