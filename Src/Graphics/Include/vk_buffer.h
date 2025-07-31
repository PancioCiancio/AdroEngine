//
// Created by apant on 29/07/2025.
//

#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include <volk/volk.h>

namespace Gfx
{

void CreateBuffer(
	VkDevice               device,
	VkPhysicalDevice       physical_device,
	VkDeviceSize           size,
	VkBufferUsageFlags     usage_flags,
	VkMemoryPropertyFlags  memory_property_flag_bits,
	VkAllocationCallbacks* p_allocator,
	VkBuffer*              p_buffer,
	VkDeviceMemory*        p_memory);

void DestroyBuffer(
	VkDevice               device,
	VkBuffer               buffer,
	VkAllocationCallbacks* p_allocator);

/// @warning	Provided buffer must be valid.
void CreateBufferView(
	VkDevice               device,
	VkBuffer               buffer,
	VkFormat               format,
	VkDeviceSize           offset,
	VkDeviceSize           range,
	VkAllocationCallbacks* p_allocator,
	VkBufferView*          p_buffer_view);

void DestroyBufferView(
	VkDevice               device,
	VkBufferView           buffer_view,
	VkAllocationCallbacks* p_allocator)
}

#endif //VK_BUFFER_H
