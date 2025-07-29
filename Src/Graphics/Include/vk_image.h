//
// Created by apant on 29/07/2025.
//

#ifndef VK_IMAGE_H
#define VK_IMAGE_H

#include <volk/volk.h>

namespace Gfx
{

void CreateImage(
	VkDevice                 device,
	VkPhysicalDevice         physical_device,
	VkImageType              image_type,
	VkFormat                 format,
	VkExtent3D               extent,
	VkSampleCountFlagBits    samples,
	VkImageTiling            tiling,
	VkImageUsageFlags        usage,
	VkMemoryPropertyFlagBits memory_property_flag_bits,
	VkAllocationCallbacks*   p_allocator,
	VkImage*                 p_image,
	VkDeviceMemory*          p_memory);

void DestroyImage(
	VkDevice               device,
	VkImage                image,
	VkAllocationCallbacks* p_allocator);

/// @warning	Provided image must be valid.
void CreateImageView(
	VkDevice               device,
	VkImage                image,
	VkImageAspectFlags     aspect_flags,
	VkImageViewType        view_type,
	VkFormat               format,
	VkComponentMapping     components,
	VkAllocationCallbacks* p_allocator,
	VkImageView*           p_image_view);

void DestroyImageView(
	VkDevice               device,
	VkImageView            image_view,
	VkAllocationCallbacks* p_allocator);
}

#endif //VK_IMAGE_H
