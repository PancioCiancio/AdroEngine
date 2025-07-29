//
// Created by apant on 29/07/2025.
//

#ifndef VK_SHADER_MODULE_H
#define VK_SHADER_MODULE_H

#include <volk/volk.h>

namespace Gfx
{
void CreateShaderModule(
	VkDevice               device,
	uint32_t               shader_code_size,
	const char*            shader_code,
	VkAllocationCallbacks* p_allocator,
	VkShaderModule*        p_shader_module);
}


#endif //VK_SHADER_MODULE_H
