//
// Created by apant on 26/07/2025.
//

#include "vk_swapchain.h"
#include "vk_utils.h"

namespace Gfx
{

void CreateSwapchain(
	VkDevice                  device,
	VkSurfaceKHR              surface,
	VkSurfaceFormatKHR*       p_format,
	VkSurfaceCapabilitiesKHR* p_capabilities,
	VkSwapchainKHR            old_swapchain,
	VkAllocationCallbacks*    p_allocator,
	VkSwapchainKHR*           p_swapchain)
{
	const VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = surface,
		.minImageCount = p_capabilities->minImageCount,
		.imageFormat = p_format->format,
		.imageColorSpace = p_format->colorSpace,
		.imageExtent = p_capabilities->currentExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = VK_TRUE,
		.oldSwapchain = old_swapchain,
	};

	VK_CHECK(vkCreateSwapchainKHR(
		device,
		&swapchain_create_info,
		p_allocator,
		p_swapchain));
}

void QuerySwapchainImages(
	VkDevice       device,
	VkSwapchainKHR swapchain,
	VkImage*       p_swapchain_images)
{
	uint32_t swapchain_image_count = 0;
	VK_CHECK(vkGetSwapchainImagesKHR(
		device,
		swapchain,
		&swapchain_image_count,
		nullptr));

	VK_CHECK(vkGetSwapchainImagesKHR(
		device,
		swapchain,
		&swapchain_image_count,
		p_swapchain_images));
}
}