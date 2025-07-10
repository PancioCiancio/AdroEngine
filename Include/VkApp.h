//
// Created by apant on 05/07/2025.
//

#ifndef VKAPP_H
#define VKAPP_H

#include <volk/volk.h>
#include <SDL2/SDL.h>
#include <vector>

class Allocator
{
public:
	/// Operator that allows an instance of this class to be used as a
	/// VkAllocationCallbacks structure
	explicit inline operator VkAllocationCallbacks()
	{
		return {
			.pUserData = static_cast<void*>(this),
			.pfnAllocation = &Allocation,
			.pfnReallocation = &Reallocation,
			.pfnFree = &Free,
			.pfnInternalAllocation = nullptr,
			.pfnInternalFree = nullptr
		};
	}

private:
	/// Declare the allocator callbacks as static member functions.

	static void* VKAPI_CALL Allocation(
		void*                   pUserData,
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocation_scope);

	static void* VKAPI_CALL Reallocation(
		void*                   pUserData,
		void*                   pOriginal,
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocation_scope);

	static void VKAPI_CALL Free(
		void* pUserData,
		void* pMemory);

	/// Now declare the nonstatic member functions that will actually perform the allocations.
	void* Allocation(
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocation_scope);

	void* Reallocation(
		void*                   pOriginal,
		size_t                  size,
		size_t                  alignment,
		VkSystemAllocationScope allocation_scope);

	void Free(
		void* pMemory);

};

class VkApp
{
public:
	VkResult Init();

	VkResult Update();

	VkResult TearDown() const;

	VkResult CreateBufferExample();

	VkResult CreateImageExample();

private:
	VkAllocationCallbacks         allocator_       = {};
	VkInstance                    instance_        = {};
	VkDebugUtilsMessengerEXT      debug_messenger_ = {};
	std::vector<VkPhysicalDevice> gpus_            = {};
	VkPhysicalDevice              gpu_             = {};
	uint8_t                       gpu_selected_    = {};
	VkDevice                      device_          = {};
	VkQueue                       queue_           = {};
	VkCommandPool                 command_pool_    = {};
	VkCommandBuffer               command_buffer_  = {};
	VkPipelineLayout              pipeline_layout_ = {};
	VkPipeline                    pipeline_        = {};

	SDL_Window*                window_                    = {};
	VkSurfaceKHR               surface_                   = {};
	VkSwapchainKHR             swapchain_                 = {};
	std::vector<VkImage>       swapchain_images_          = {};
	std::vector<VkImageView>   swapchain_image_views_     = {};
	VkSemaphore                image_available_semaphore_ = {};
	VkSemaphore                render_finished_semaphore_ = {};
	VkFence                    submit_finished_fence_     = {};
	VkRenderPass               render_pass_               = {};
	std::vector<VkFramebuffer> framebuffers_              = {};

	VkImage        framebuffer_sample_image_        = {};
	VkImageView    framebuffer_sample_image_view_   = {};
	VkDeviceMemory framebuffer_sample_image_memory_ = {};
};

#endif //VKAPP_H