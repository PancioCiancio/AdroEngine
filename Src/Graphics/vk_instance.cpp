//
// Created by apant on 25/07/2025.
//

#include "vk_instance.h"
#include "vk_utils.h"

// std
#include <stdlib.h>


namespace Gfx
{
void CreateInstance(
	uint32_t               requested_layer_count,
	const char**           p_requested_layers,
	uint32_t               requested_extension_count,
	const char**           p_requested_extensions,
	VkAllocationCallbacks* p_allocator,
	VkInstance*            p_instance)
{

	uint32_t layer_count = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(
		&layer_count,
		nullptr));

	VkLayerProperties layer_properties[256] = {};
	VK_CHECK(vkEnumerateInstanceLayerProperties(
		&layer_count,
		&layer_properties[0]));

	uint32_t extension_count = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extension_count,
		nullptr));

	VkExtensionProperties extension_properties[256] = {};
	VK_CHECK(vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extension_count,
		&extension_properties[0]));

	// @todo	check for layer and extensions required.
	// @todo	enaleble debug messenger
	//const VkDebugUtilsMessengerCreateInfoEXT debug_create_info_ext = GetDebugMessengerCreateInfo();

	const VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Adro",
		.applicationVersion = 1,
		.apiVersion = VK_MAKE_VERSION(1, 0, 0)
	};

	const VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		// @todo	point to the debug messenger
		.pNext = nullptr, //&debug_create_info_ext,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = requested_layer_count,
		.ppEnabledLayerNames = &p_requested_layers[0],
		.enabledExtensionCount = requested_extension_count,
		.ppEnabledExtensionNames = &p_requested_extensions[0],
	};

	VK_CHECK(vkCreateInstance(
		&create_info,
		p_allocator,
		p_instance));
}

void DestroyInstance(
	VkInstance             instance,
	VkAllocationCallbacks* p_allocator)
{
	vkDestroyInstance(
		instance,
		p_allocator);
}
}

