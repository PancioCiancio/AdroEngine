//
// Created by apant on 26/07/2025.
//

#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

#include <volk/volk.h>

namespace Gfx
{
void CreateSwapchain(
	VkDevice                  device,
	VkSurfaceKHR              surface,
	VkSurfaceFormatKHR*       p_format,
	VkSurfaceCapabilitiesKHR* p_capabilities,
	VkSwapchainKHR            old_swapchain,
	VkAllocationCallbacks*    p_allocator,
	VkSwapchainKHR*           p_swapchain);

void DestroySwapchain(
	VkDevice               device,
	VkSwapchainKHR         swapchain,
	VkAllocationCallbacks* p_allocator);

/// @param p_swapchain_images the size must be VkSurfaceCapabilitiesKHR::minImageCount
///		   You can query such image count by calling QuerySurfaceCapabilities.
void QuerySwapchainImages(
	VkDevice       device,
	VkSwapchainKHR swapchain,
	VkImage*       p_swapchain_images);
}

#endif //VK_SWAPCHAIN_H
