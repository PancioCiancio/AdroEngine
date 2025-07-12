//
// Created by apant on 07/07/2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Volk/volk.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	/// Mostly used to group swapchain images, views and framebuffers
	/// @todo Presentation should be separated from base Graphics
	struct PerFrame
	{
		std::vector<VkImage>       images;
		std::vector<VkImageView>   image_views;
		std::vector<VkFramebuffer> framebuffers;
	};

	/// Uniform buffer data aligned
	struct PerFrameData
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
	};

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
	};

	static void CreateInstance(
		const std::vector<const char*>& layers_requested,
		const std::vector<const char*>& extensions_requested,
		VkAllocationCallbacks*          p_allocator,
		VkInstance*                     p_instance);

	static VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

	static void CreateDebugMessenger(
		VkInstance                instance,
		VkAllocationCallbacks*    p_allocator,
		VkDebugUtilsMessengerEXT* p_debuge_messenger_ext);

	static void QueryGpu(
		VkInstance                      instance,
		VkPhysicalDeviceFeatures        fts_requested,
		const std::vector<const char*>& extensions_requested,
		VkPhysicalDevice*               p_gpu);

	static void QuerySampleCounts(
		VkPhysicalDevice       gpu,
		VkSampleCountFlagBits* p_sample);

	/// @todo Platform dependent. Consider to solve the platform implementation at static link time.
	static void QueryQueueFamily(
		VkPhysicalDevice gpu,
		VkQueueFlagBits  queue_flag_bits_requested,
		bool             must_support_presentation,
		uint32_t         family_idx_discarded_count,
		const uint32_t*  family_idx_discarded,
		uint32_t*        p_queue_family_idx);

	static void CreateDevice(
		VkPhysicalDevice                gpu,
		uint32_t                        queue_family_count,
		const uint32_t*                 p_queue_families_idx,
		const std::vector<const char*>& extensions,
		const VkPhysicalDeviceFeatures* p_features,
		const VkAllocationCallbacks*    p_allocator,
		VkDevice*                       p_device);

	static void QuerySurfaceFormat(
		VkPhysicalDevice             gpu,
		VkSurfaceKHR                 surface,
		const std::vector<VkFormat>& formats_required,
		VkSurfaceFormatKHR*          p_format);

	/// Query surface capabilities and fix image count and transform.
	static void QuerySurfaceCapabilities(
		VkPhysicalDevice          gpu,
		VkSurfaceKHR              surface,
		VkSurfaceCapabilitiesKHR* p_surface_capabilities);

	static void CreateSwapchain(
		VkDevice                  device,
		VkSurfaceKHR              surface,
		VkSurfaceFormatKHR*       p_format,
		VkSurfaceCapabilitiesKHR* p_capabilities,
		VkSwapchainKHR            old_swapchain,
		VkAllocationCallbacks*    p_allocator,
		VkSwapchainKHR*           p_swapchain);

	/// @param p_swapchain_images the size must be VkSurfaceCapabilitiesKHR::minImageCount
	///		   You can query such image count by calling QuerySurfaceCapabilities.
	static void QuerySwapchainImages(
		VkDevice       device,
		VkSwapchainKHR swapchain,
		VkImage*       p_swapchain_images);

	static void CreateCommandPool(
		VkDevice                 device,
		VkCommandPoolCreateFlags flags,
		uint32_t                 queue_family_idx,
		VkAllocationCallbacks*   p_allocator,
		VkCommandPool*           p_command_pool);

	static void CreateCommandBuffer(
		VkDevice         device,
		VkCommandPool    command_pool,
		VkCommandBuffer* p_command_buffer);

	static void CreateSemaphore(
		VkDevice               device,
		VkAllocationCallbacks* p_allocator,
		VkSemaphore*           p_semaphore);

	static void CreateFence(
		VkDevice               device,
		VkAllocationCallbacks* p_allocator,
		VkFence*               p_fence);

	/// @todo Sketch implementation. Refactor it...
	static uint32_t QueryMemoryTypeIdx(
		uint32_t                                type_filter,
		VkMemoryPropertyFlags                   memory_property_flags,
		const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties);

	static void CreateShaderModule(
		VkDevice                 device,
		const std::vector<char>& shader_code,
		VkAllocationCallbacks*   p_allocator,
		VkShaderModule*          p_shader_module);

	static void CreateBuffer(
		VkDevice               device,
		VkPhysicalDevice       physical_device,
		VkDeviceSize           size,
		VkBufferUsageFlags     usage_flags,
		VkMemoryPropertyFlags  memory_property_flag_bits,
		VkAllocationCallbacks* p_allocator,
		VkBuffer*              p_buffer,
		VkDeviceMemory*        p_memory);

	/// @warning	Provided buffer must be valid.
	static void CreateBufferView(
		VkDevice               device,
		VkBuffer               buffer,
		VkFormat               format,
		VkDeviceSize           offset,
		VkDeviceSize           range,
		VkAllocationCallbacks* p_allocator,
		VkBufferView*          p_buffer_view);

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