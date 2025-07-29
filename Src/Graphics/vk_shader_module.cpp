//
// Created by apant on 29/07/2025.
//

#include "Include/vk_shader_module.h"
#include "Include/vk_utils.h"

namespace Gfx
{
void CreateShaderModule(
	VkDevice               device,
	uint32_t               shader_code_size,
	const char*            shader_code,
	VkAllocationCallbacks* p_allocator,
	VkShaderModule*        p_shader_module)
{
	const VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = shader_code_size,
		.pCode = reinterpret_cast<const uint32_t*>(&shader_code[0]),
	};

	VK_CHECK(vkCreateShaderModule(
		device,
		&create_info,
		p_allocator,
		p_shader_module));
}
}