//
// Created by apant on 07/07/2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Volk/volk.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Graphics {
public:
	Graphics() = delete;

	/// Mostly used to group swapchain images, views and framebuffers
	/// @todo Presentation should be separated from base Graphics
	struct PerFrame {
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;
		std::vector<VkFramebuffer> framebuffers;
	};

	/// Uniform buffer data aligned
	struct PerFrameData {
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
	};

	static void CreateSwapchain(
		VkDevice device,
		VkSurfaceKHR surface,
		VkSurfaceFormatKHR *p_format,
		VkSurfaceCapabilitiesKHR *p_capabilities,
		VkSwapchainKHR old_swapchain,
		VkAllocationCallbacks *p_allocator,
		VkSwapchainKHR *p_swapchain);

	/// @param p_swapchain_images the size must be VkSurfaceCapabilitiesKHR::minImageCount
	///		   You can query such image count by calling QuerySurfaceCapabilities.
	static void QuerySwapchainImages(
		VkDevice device,
		VkSwapchainKHR swapchain,
		VkImage *p_swapchain_images);

	static void CreateCommandPool(
		VkDevice device,
		VkCommandPoolCreateFlags flags,
		uint32_t queue_family_idx,
		VkAllocationCallbacks *p_allocator,
		VkCommandPool *p_command_pool);

	static void CreateCommandBuffer(
		VkDevice device,
		VkCommandPool command_pool,
		VkCommandBuffer *p_command_buffer);

	static void CreateSemaphore(
		VkDevice device,
		VkAllocationCallbacks *p_allocator,
		VkSemaphore *p_semaphore);

	static void CreateFence(
		VkDevice device,
		VkAllocationCallbacks *p_allocator,
		VkFence *p_fence);

	/// @todo Sketch implementation. Refactor it...
	static uint32_t QueryMemoryTypeIdx(
		uint32_t type_filter,
		VkMemoryPropertyFlags memory_property_flags,
		const VkPhysicalDeviceMemoryProperties &physical_device_memory_properties);

	static void CreateShaderModule(
		VkDevice device,
		const std::vector<char> &shader_code,
		VkAllocationCallbacks *p_allocator,
		VkShaderModule *p_shader_module);

	static void CreateBuffer(
		VkDevice device,
		VkPhysicalDevice physical_device,
		VkDeviceSize size,
		VkBufferUsageFlags usage_flags,
		VkMemoryPropertyFlags memory_property_flag_bits,
		VkAllocationCallbacks *p_allocator,
		VkBuffer *p_buffer,
		VkDeviceMemory *p_memory);

	/// @warning	Provided buffer must be valid.
	static void CreateBufferView(
		VkDevice device,
		VkBuffer buffer,
		VkFormat format,
		VkDeviceSize offset,
		VkDeviceSize range,
		VkAllocationCallbacks *p_allocator,
		VkBufferView *p_buffer_view);

	static void CreateImage(
		VkDevice device,
		VkPhysicalDevice physical_device,
		VkImageType image_type,
		VkFormat format,
		VkExtent3D extent,
		VkSampleCountFlagBits samples,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlagBits memory_property_flag_bits,
		VkAllocationCallbacks *p_allocator,
		VkImage *p_image,
		VkDeviceMemory *p_memory);

	/// @warning	Provided image must be valid.
	static void CreateImageView(
		VkDevice device,
		VkImage image,
		VkImageAspectFlags aspect_flags,
		VkImageViewType view_type,
		VkFormat format,
		VkComponentMapping components,
		VkAllocationCallbacks *p_allocator,
		VkImageView *p_image_view);
};

#endif //GRAPHICS_H
