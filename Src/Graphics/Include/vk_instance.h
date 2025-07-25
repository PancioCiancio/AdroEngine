//
// Created by apant on 25/07/2025.
//

#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

#include <volk/volk.h>

namespace Gfx {
	void CreateInstance(
		uint32_t requested_layer_count,
		const char **p_requested_layers,
		uint32_t requested_extension_count,
		const char **p_requested_extensions,
		VkAllocationCallbacks *p_allocator,
		VkInstance *p_instance);
}

#endif //VK_INSTANCE_H
