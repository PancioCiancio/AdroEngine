//
// Created by apant on 25/07/2025.
//

#ifndef VK_DEBUG_MESSENGER_H
#define VK_DEBUG_MESSENGER_H

#include <volk/volk.h>

namespace Gfx
{
// @todo	create wrapper class for the debug callback to allow passing the pUserData and use the logger
//			chosen by the client.
VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

void CreateDebugMessenger(
	VkInstance                instance,
	VkAllocationCallbacks*    p_allocator,
	VkDebugUtilsMessengerEXT* p_debuge_messenger_ext);

void DestroyDebugMessenger(
	VkInstance               instance,
	VkDebugUtilsMessengerEXT debug_messenger,
	VkAllocationCallbacks*   p_allocator);
}

#endif //VK_DEBUG_MESSENGER_H
