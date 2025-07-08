//
// Created by apant on 07/07/2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Volk/volk.h>
#include <vector>

class Graphics
{
public:
	Graphics() = delete;

	/// Debug mode only check.
#define VK_CHECK(result)					\
	do {                                    \
		VkResult _vk_result = (result);     \
		assert(_vk_result == VK_SUCCESS);	\
	} while (0)

	/// Create graphics instance into the provided ptr.
	/// @warning First call VolkInitialize (if using volk library).
	static void CreateInstance(
		const std::vector<const char*>& layers_requested,
		const std::vector<const char*>& extensions_requested,
		VkAllocationCallbacks*          p_allocator,
		VkInstance*                     p_instance);

	static VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

	/// Create debug messenger into the provided ptr.
	/// @warning First call volkLoadInstance (if using volk library).
	static void CreateDebugMessenger(
		VkInstance                instance,
		VkAllocationCallbacks*    p_allocator,
		VkDebugUtilsMessengerEXT* p_debuge_messenger_ext);

	/// Return the gpu supporting the requested features.
	/// @warning If any gpu doesn't support the requested features and extensions, the program is alt.
	static void SelectGpu(
		VkInstance                      instance,
		VkPhysicalDeviceFeatures        fts_requested,
		const std::vector<const char*>& extensions_requested,
		VkPhysicalDevice*               p_gpu);

	/// Provide the suitable memory type to bind resources.
	/// @todo Sketch implementation. Refactor it...
	static uint32_t FindMemoryTypeIdx(
		uint32_t                                type_filter,
		VkMemoryPropertyFlags                   memory_property_flags,
		const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties);

	/// Create the shader module from the provided shader program into the given ptr.
	static void CreateShaderModule(
		VkDevice                 device,
		const std::vector<char>& shader_code,
		VkAllocationCallbacks*   p_allocator,
		VkShaderModule*          p_shader_module);

	/// Create a sequential buffer into the given ptr and bind the memory to it.
	static void CreateBuffer(
		VkDevice                 device,
		VkPhysicalDevice         physical_device,
		VkDeviceSize             size,
		VkBufferUsageFlags       usage_flags,
		VkFormat                 format,
		VkMemoryPropertyFlagBits memory_property_flag_bits,
		VkAllocationCallbacks*   p_allocator,
		VkBuffer*                p_buffer,
		VkDeviceMemory*          p_memory);

	/// Create a view into the given buffer.
	/// @warning	Provided buffer must be valid.
	static void CreateBufferView(
		VkDevice               device,
		VkBuffer               buffer,
		VkFormat               format,
		VkDeviceSize           offset,
		VkDeviceSize           range,
		VkAllocationCallbacks* p_allocator,
		VkBufferView*          p_buffer_view);

	/// Create an image into the given ptr and bind the memory to it.
	static void CreateImage(
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

	/// Create a view into the given image.
	/// @warning	Provided image must be valid.
	static void CreateImageView(
		VkDevice               device,
		VkImage                image,
		VkImageViewType        view_type,
		VkFormat               format,
		VkComponentMapping     components,
		VkAllocationCallbacks* p_allocator,
		VkImageView*           p_image_view);
};

#endif //GRAPHICS_H