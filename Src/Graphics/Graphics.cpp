//
// Created by apant on 07/07/2025.
//

#include "Graphics.h"
#include "vk_utils.h"

#include <cassert>
#include <stdexcept>
#include <cstring>

void Graphics::CreateSwapchain(
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

void Graphics::QuerySwapchainImages(
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

void Graphics::CreateCommandPool(
	VkDevice                 device,
	VkCommandPoolCreateFlags flags,
	uint32_t                 queue_family_idx,
	VkAllocationCallbacks*   p_allocator,
	VkCommandPool*           p_command_pool)
{
	const VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = flags,
		.queueFamilyIndex = 0,
	};

	VK_CHECK(vkCreateCommandPool(
		device,
		&command_pool_create_info,
		p_allocator,
		p_command_pool));
}

void Graphics::CreateCommandBuffer(
	VkDevice         device,
	VkCommandPool    command_pool,
	VkCommandBuffer* p_command_buffer)
{
	const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VK_CHECK(vkAllocateCommandBuffers(
		device,
		&command_buffer_allocate_info,
		p_command_buffer));
}

void Graphics::CreateSemaphore(
	VkDevice               device,
	VkAllocationCallbacks* p_allocator,
	VkSemaphore*           p_semaphore)
{
	VkSemaphoreCreateInfo image_available_semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
	};

	VK_CHECK(vkCreateSemaphore(
		device,
		&image_available_semaphore_info,
		p_allocator,
		p_semaphore));
}

void Graphics::CreateFence(
	VkDevice               device,
	VkAllocationCallbacks* p_allocator,
	VkFence*               p_fence)
{
	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	VK_CHECK(vkCreateFence(
		device,
		&fence_info,
		p_allocator,
		p_fence));
}

uint32_t Graphics::QueryMemoryTypeIdx(
	uint32_t                                type_filter,
	VkMemoryPropertyFlags                   memory_property_flags,
	const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties)
{
	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		const bool is_type_supported       = (type_filter & (1 << i)) != 0;
		const bool has_required_properties = (physical_device_memory_properties.memoryTypes[i].propertyFlags &
		                                      memory_property_flags)
		                                     == memory_property_flags;

		if (is_type_supported && has_required_properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

void Graphics::CreateShaderModule(
	VkDevice                 device,
	const std::vector<char>& shader_code,
	VkAllocationCallbacks*   p_allocator,
	VkShaderModule*          p_shader_module)
{
	const VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = static_cast<uint32_t>(shader_code.size()),
		.pCode = reinterpret_cast<const uint32_t*>(&shader_code[0]),
	};

	VK_CHECK(vkCreateShaderModule(
		device,
		&create_info,
		p_allocator,
		p_shader_module));
}

void Graphics::CreateBuffer(
	VkDevice               device,
	VkPhysicalDevice       physical_device,
	VkDeviceSize           size,
	VkBufferUsageFlags     usage_flags,
	VkMemoryPropertyFlags  memory_property_flag_bits,
	VkAllocationCallbacks* p_allocator,
	VkBuffer*              p_buffer,
	VkDeviceMemory*        p_memory)
{
	const VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = size,
		.usage = usage_flags,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
	};

	VK_CHECK(vkCreateBuffer(
		device,
		&buffer_create_info,
		p_allocator,
		p_buffer));

	VkMemoryRequirements mem_requirements = {};
	vkGetBufferMemoryRequirements(
		device,
		*p_buffer,
		&mem_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(
		physical_device,
		&memory_properties);

	const uint32_t memory_type_index = QueryMemoryTypeIdx(
		mem_requirements.memoryTypeBits,
		memory_property_flag_bits,
		memory_properties);

	const VkMemoryAllocateInfo buffer_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = memory_type_index
	};

	VK_CHECK(vkAllocateMemory(device, &buffer_memory_allocate_info, p_allocator, p_memory));
	VK_CHECK(vkBindBufferMemory(device, *p_buffer, *p_memory, 0));
}

void Graphics::CreateBufferView(
	VkDevice               device,
	VkBuffer               buffer,
	VkFormat               format,
	VkDeviceSize           offset,
	VkDeviceSize           range,
	VkAllocationCallbacks* p_allocator,
	VkBufferView*          p_buffer_view)
{
	const VkBufferViewCreateInfo buffer_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.buffer = buffer,
		.format = format,
		.offset = offset,
		.range = range,
	};

	VK_CHECK(vkCreateBufferView(
		device,
		&buffer_view_create_info,
		p_allocator,
		p_buffer_view));
}

void Graphics::CreateImage(
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

	const uint32_t mem_type_idx = QueryMemoryTypeIdx(
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

void Graphics::CreateImageView(
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