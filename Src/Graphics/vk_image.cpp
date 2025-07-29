//
// Created by apant on 29/07/2025.
//

#include "vk_image.h"
#include "vk_utils.h"

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
	VkDeviceMemory*          p_memory)
{
	const VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = image_type,
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = samples,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VK_CHECK(vkCreateImage(
		device,
		&image_info,
		nullptr,
		p_image));

	VkMemoryRequirements mem_requirements = {};
	vkGetImageMemoryRequirements(device, *p_image, &mem_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	const uint32_t mem_type_idx = Gfx::QueryMemoryTypeIdx(
		mem_requirements.memoryTypeBits,
		memory_property_flag_bits,
		memory_properties);

	const VkMemoryAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = mem_type_idx,
	};

	VK_CHECK(vkAllocateMemory(
		device,
		&allocate_info,
		p_allocator,
		p_memory));

	VK_CHECK(vkBindImageMemory(
		device,
		*p_image,
		*p_memory,
		0));
}

void DestroyImage(
	VkDevice               device,
	VkImage                image,
	VkAllocationCallbacks* p_allocator)
{
	vkDestroyImage(
		device,
		image,
		p_allocator);
}

void CreateImageView(
	VkDevice               device,
	VkImage                image,
	VkImageAspectFlags     aspect_flags,
	VkImageViewType        view_type,
	VkFormat               format,
	VkComponentMapping     components,
	VkAllocationCallbacks* p_allocator,
	VkImageView*           p_image_view)
{
	const VkImageSubresourceRange image_subresource_range = {
		.aspectMask = aspect_flags,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	const VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = image,
		.viewType = view_type,
		.format = format,
		.components = components,
		.subresourceRange = image_subresource_range,
	};

	VK_CHECK(vkCreateImageView(
		device,
		&image_view_create_info,
		p_allocator,
		p_image_view));
}

void DestroyImageView(
	VkDevice               device,
	VkImageView            image_view,
	VkAllocationCallbacks* p_allocator)
{
	vkDestroyImageView(
		device,
		image_view,
		p_allocator);
}
}