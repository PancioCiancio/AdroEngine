//
// Created by apant on 05/07/2025.
//

#include "VkApp.h"

#include <cassert>
#include <queue>
#include <iostream>

void* Allocator::Allocation(
	void*                   pUserData,
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocation_scope)
{
	return static_cast<Allocator*>(pUserData)->Allocation(size, alignment, allocation_scope);
}

void* Allocator::Reallocation(
	void*                   pUserData,
	void*                   pOriginal,
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocation_scope)
{
	return static_cast<Allocator*>(pUserData)->Reallocation(pOriginal, size, alignment, allocation_scope);
}

void Allocator::Free(
	void* pUserData,
	void* pMemory)
{
	return static_cast<Allocator*>(pUserData)->Free(pMemory);
}

void* Allocator::Allocation(
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocation_scope)
{
	return _aligned_malloc(
		size,
		alignment);
}

void* Allocator::Reallocation(
	void*                   pOriginal,
	size_t                  size,
	size_t                  alignment,
	VkSystemAllocationScope allocation_scope)
{
	return _aligned_realloc(pOriginal, size, alignment);
}

void Allocator::Free(
	void* pMemory)
{
	return _aligned_free(pMemory);
}


inline void VK_CHECK(const VkResult result)
{
	assert(result == VK_SUCCESS);
}

VkResult VkApp::Init()
{
	VK_CHECK(volkInitialize());

	// Create the custom allocator
	allocator_ = static_cast<VkAllocationCallbacks>(Allocator());

	// Query instance properties.
	uint32_t instance_layer_count = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(
		&instance_layer_count,
		nullptr));

	std::vector<VkLayerProperties> instance_layer_properties(instance_layer_count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(
		&instance_layer_count,
		&instance_layer_properties[0]));

	// Query instance extensions.
	uint32_t instance_extension_count = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(
		nullptr,
		&instance_extension_count,
		nullptr));

	std::vector<VkExtensionProperties> instance_extension_properties(instance_extension_count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(
		nullptr,
		&instance_extension_count,
		&instance_extension_properties[0]));

	// A generic application info structure
	const VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Adro",
		.applicationVersion = 1,
		.apiVersion = VK_MAKE_API_VERSION(1, 0, 0, 0)
	};

	// Create the instance
	const VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info
	};

	VK_CHECK(vkCreateInstance(
		&create_info,
		&allocator_,
		&instance_));

	volkLoadInstance(instance_);

	// First figure out how many devices are in the system
	uint32_t gpu_count = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(
		instance_,
		&gpu_count,
		nullptr));

	// Size the device array appropriately and get the physical device handles
	gpus_.resize(gpu_count);
	VK_CHECK(vkEnumeratePhysicalDevices(
		instance_,
		&gpu_count,
		&gpus_[0]));

	// List the required gpu features
	const VkPhysicalDeviceFeatures gpu_required_features = {
		.geometryShader = VK_TRUE,
		.tessellationShader = VK_TRUE,
		.multiDrawIndirect = VK_TRUE,
	};

	// Select gpu algorithm.
	for (size_t i = 0; i < gpus_.size(); i++)
	{
		const VkPhysicalDevice gpu = gpus_[i];

		VkPhysicalDeviceProperties gpu_properties = {};
		vkGetPhysicalDeviceProperties(gpu, &gpu_properties);

		// You might need to check format properties as well
		// vkGetPhysicalDeviceFormatProperties(gpu, )

		// You might need to check image properties as well
		// vkGetPhysicalDeviceImageFormatProperties(gpu)

		// Query physical device extensions.
		uint32_t gpu_extension_count = 0;
		VK_CHECK(vkEnumerateDeviceExtensionProperties(
			gpu,
			nullptr,
			&gpu_extension_count,
			nullptr));

		std::vector<VkExtensionProperties> gpu_extension_propertieses(gpu_extension_count);
		VK_CHECK(vkEnumerateDeviceExtensionProperties(
			gpu,
			nullptr,
			&gpu_extension_count,
			&gpu_extension_propertieses[0]));

		if (gpu_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			gpu_selected_ = i;
		}

		// Check also if the features required are supported...
	}

	const VkDeviceQueueCreateInfo queue_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.flags = 0,
		.queueFamilyIndex = 0,
		.queueCount = 1,
		.pQueuePriorities = nullptr
	};

	const VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_create_info,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
		.pEnabledFeatures = &gpu_required_features
	};

	VK_CHECK(vkCreateDevice(
		gpus_[gpu_selected_],
		&device_create_info,
		&allocator_,
		&device_));

	return VK_SUCCESS;
}

VkResult VkApp::TearDown() const
{
	VK_CHECK(vkDeviceWaitIdle(device_));

	vkDestroyDevice(device_, &allocator_);
	vkDestroyInstance(instance_, &allocator_);

	return VK_SUCCESS;
}

uint32_t FindMemoryTypeIndex(
	uint32_t typeFilter,
	// From memory requirements (memoryTypeBits)
	VkMemoryPropertyFlags properties,
	// Required memory properties
	const VkPhysicalDeviceMemoryProperties& memProperties)
{
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		bool is_type_supported       = (typeFilter & (1 << i)) != 0;
		bool has_required_properties = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;

		if (is_type_supported && has_required_properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

VkResult VkApp::CreateBufferExample()
{
	/// Show an example of how to create a buffer.

	// First, create the buffer
	const VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = 1024 * 1024,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
	};

	VkBuffer buffer = {};
	VK_CHECK(vkCreateBuffer(device_, &buffer_create_info, &allocator_, &buffer));

	// Create buffer view
	const VkBufferViewCreateInfo buffer_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.buffer = buffer,
		.format = VK_FORMAT_R8_SINT,
		.offset = 0,
		.range = 1024 * 1024,
	};

	VkBufferView buffer_view = {};
	VK_CHECK(vkCreateBufferView(device_, &buffer_view_create_info, &allocator_, &buffer_view));

	// Second, bake to the memory
	VkMemoryRequirements buffer_memory_requirements = {};
	vkGetBufferMemoryRequirements(device_, buffer, &buffer_memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(gpus_[gpu_selected_], &memory_properties);

	const uint32_t memory_type_index = FindMemoryTypeIndex(
		buffer_memory_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		memory_properties);

	const VkMemoryAllocateInfo buffer_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = buffer_memory_requirements.size,
		.memoryTypeIndex = memory_type_index
	};

	VkDeviceMemory buffer_memory = {};
	VK_CHECK(vkAllocateMemory(device_, &buffer_memory_allocate_info, &allocator_, &buffer_memory));
	VK_CHECK(vkBindBufferMemory(device_, buffer, buffer_memory, 0));

	return VK_SUCCESS;
}

VkResult VkApp::CreateImageExample()
{
	const VkImageCreateInfo image_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_UNORM,
		.extent = {1024, 1024, 1},
		.mipLevels = 10,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkImage image = {};
	VK_CHECK(vkCreateImage(device_, &image_create_info, &allocator_, &image));

	// Create image view
	const VkImageSubresourceRange image_subresource_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
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
		.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
		.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM,
		.components = VK_COMPONENT_SWIZZLE_IDENTITY,
		.subresourceRange = image_subresource_range,
	};

	VkImageView image_view = {};
	VK_CHECK(vkCreateImageView(device_, &image_view_create_info, &allocator_, &image_view));

	VkMemoryRequirements image_memory_requirements = {};
	vkGetImageMemoryRequirements(device_, image, &image_memory_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(gpus_[gpu_selected_], &memory_properties);

	const uint32_t memory_type_index = FindMemoryTypeIndex(
		image_memory_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		memory_properties);

	const VkMemoryAllocateInfo memory_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = image_memory_requirements.size,
		.memoryTypeIndex = memory_type_index
	};

	VkDeviceMemory image_memory = {};
	VK_CHECK(vkAllocateMemory(device_, &memory_memory_allocate_info, &allocator_, &image_memory));
	VK_CHECK(vkBindImageMemory(device_, image, image_memory, 0));

	return VK_SUCCESS;
}