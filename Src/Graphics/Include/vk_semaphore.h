//
// Created by apant on 26/07/2025.
//

#ifndef VK_SEMAPHORE_H
#define VK_SEMAPHORE_H


#include <volk/volk.h>

namespace Gfx
{
void CreateSemaphore(
	VkDevice               device,
	VkAllocationCallbacks* p_allocator,
	VkSemaphore*           p_semaphore);
}


#endif //VK_SEMAPHORE_H
