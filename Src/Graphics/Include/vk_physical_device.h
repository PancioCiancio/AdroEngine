//
// Created by apant on 25/07/2025.
//

#ifndef VK_PHYSICAL_DEVICE_H
#define VK_PHYSICAL_DEVICE_H

#include <volk/volk.h>

namespace Gfx {
	void QueryGpu(
		VkInstance instance,
		VkPhysicalDeviceFeatures fts_requested,
		uint32_t requested_extension_count,
		const char **p_requested_extensions,
		VkPhysicalDevice *p_gpu);

	void QuerySampleCounts(
		VkPhysicalDevice gpu,
		VkSampleCountFlagBits *p_sample);

	/// @todo Platform dependent. Consider to solve the platform implementation at static link time.
	void QueryQueueFamily(
		VkPhysicalDevice gpu,
		VkQueueFlagBits queue_flag_bits_requested,
		bool must_support_presentation,
		uint32_t family_idx_discarded_count,
		const uint32_t *family_idx_discarded,
		uint32_t *p_queue_family_idx);

	void QuerySurfaceFormat(
		VkPhysicalDevice gpu,
		VkSurfaceKHR surface,
		uint32_t required_format_count,
		const VkFormat *p_required_formats,
		VkSurfaceFormatKHR *p_format);

	/// Query surface capabilities and fix image count and transform.
	void QuerySurfaceCapabilities(
		VkPhysicalDevice gpu,
		VkSurfaceKHR surface,
		VkSurfaceCapabilitiesKHR *p_surface_capabilities);
}


#endif //VK_PHYSICAL_DEVICE_H
