//
// Created by apant on 25/07/2025.
//

#ifndef VK_UTILS_H
#define VK_UTILS_H

// vulkan
#include <volk/volk.h>

// std
#include <cassert>

namespace Gfx {
	// @todo	do not define this here. Provide a way to communicate the error to the client.
#define VK_CHECK(result)					\
	do {                                    \
		VkResult _vk_result = (result);     \
		assert(_vk_result == VK_SUCCESS);	\
	} while (0)


	/// @todo Sketch implementation. Refactor it...
	uint32_t QueryMemoryTypeIdx(
		uint32_t type_filter,
		VkMemoryPropertyFlags memory_property_flags,
		const VkPhysicalDeviceMemoryProperties &physical_device_memory_properties);
}

#endif //VK_UTILS_H
