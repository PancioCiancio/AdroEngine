//
// Created by apant on 25/07/2025.
//

#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include <volk/volk.h>

namespace Gfx {
	void CreateDevice(
		VkPhysicalDevice gpu,
		uint32_t queue_family_count,
		const uint32_t *p_queue_families_idx,
		uint32_t requested_extension_count,
		const char **p_requested_extensions,
		const VkPhysicalDeviceFeatures *p_features,
		const VkAllocationCallbacks *p_allocator,
		VkDevice *p_device);
}

#endif //VK_DEVICE_H
