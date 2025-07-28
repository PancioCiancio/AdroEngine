//
// Created by apant on 26/07/2025.
//

#ifndef VK_COMMAND_BUFFER_H
#define VK_COMMAND_BUFFER_H

#include <volk/volk.h>

namespace Gfx
{


void CreateCommandBuffer(
	VkDevice         device,
	VkCommandPool    command_pool,
	VkCommandBuffer* p_command_buffer);

}


#endif //VK_COMMAND_BUFFER_H
