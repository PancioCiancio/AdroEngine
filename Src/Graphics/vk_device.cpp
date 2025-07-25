//
// Created by apant on 25/07/2025.
//

#include "vk_device.h"
#include "vk_utils.h"

namespace Gfx
{
void CreateDevice(
	VkPhysicalDevice                gpu,
	const uint32_t                  queue_family_count,
	const uint32_t*                 p_queue_families_idx,
	uint32_t                        requested_extension_count,
	const char**                    p_requested_extensions,
	const VkPhysicalDeviceFeatures* p_features,
	const VkAllocationCallbacks*    p_allocator,
	VkDevice*                       p_device)
{
	VkDeviceQueueCreateInfo queue_create_infos[32] = {};

	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		constexpr float queue_priorities = 1.0f;

		const VkDeviceQueueCreateInfo queue_create_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.flags = 0,
			.queueFamilyIndex = p_queue_families_idx[i],
			.queueCount = 1,
			.pQueuePriorities = &queue_priorities,
		};

		queue_create_infos[i] = queue_create_info;
	}

	const VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.flags = 0,
		.queueCreateInfoCount = queue_family_count,
		.pQueueCreateInfos = &queue_create_infos[0],
		.enabledExtensionCount = requested_extension_count,
		.ppEnabledExtensionNames = &p_requested_extensions[0],
		.pEnabledFeatures = p_features
	};

	VK_CHECK(vkCreateDevice(
		gpu,
		&device_create_info,
		p_allocator,
		p_device));
}
}
