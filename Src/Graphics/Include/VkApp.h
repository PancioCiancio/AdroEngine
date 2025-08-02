//
// Created by apant on 05/07/2025.
//

#ifndef VKAPP_H
#define VKAPP_H

#include <volk/volk.h>
#include <SDL2/SDL.h>
#include <vector>

#include "Graphics.h"


/// Groups of all scene vertex data.
struct Batch
{
	std::vector<glm::vec3> position;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> color;
	std::vector<uint32_t>  indices;
};

/// Packs all buffer and memory used for graphics.
struct BatchRender
{
	VkBuffer       position_buffer = {};
	VkDeviceMemory position_memory = {};

	VkBuffer       normal_buffer = {};
	VkDeviceMemory normal_memory = {};

	VkBuffer       color_buffer = {};
	VkDeviceMemory color_memory = {};

	VkBuffer       index_buffer = {};
	VkDeviceMemory index_memory = {};
};

class VkApp
{
public:
	void Init();

	void Update();

	void TearDown() const;

private:
	VkAllocationCallbacks         allocator_          = {};
	VkInstance                    instance_           = {};
	VkDebugUtilsMessengerEXT      debug_messenger_    = {};
	std::vector<VkPhysicalDevice> gpus_               = {};
	VkPhysicalDevice              gpu_                = {};
	VkDevice                      device_             = {};
	VkQueue                       queue_              = {};
	VkCommandPool                 command_pool_       = {};
	VkCommandBuffer               command_buffer_     = {};
	VkPipelineLayout              pipeline_layout_    = {};
	VkPipeline                    pipeline_           = {};
	VkPipeline                    pipeline_wireframe_ = {};

	SDL_Window*    window_                    = {};
	VkSurfaceKHR   surface_                   = {};
	VkSwapchainKHR swapchain_                 = {};
	VkSemaphore    image_available_semaphore_ = {};
	VkSemaphore    render_finished_semaphore_ = {};
	VkFence        submit_finished_fence_     = {};
	VkRenderPass   render_pass_               = {};

	Graphics::PerFrame presentation_frames_ = {};

	std::vector<VkBuffer>        per_frame_data_buffers_  = {};
	std::vector<VkDeviceMemory>  per_frame_data_memories_ = {};
	std::vector<void*>           per_frame_data_mapped_   = {};
	VkDescriptorSetLayout        descriptor_set_layout_   = {};
	VkDescriptorPool             descriptor_pool_         = {};
	std::vector<VkDescriptorSet> descriptor_sets_         = {};

	VkImage        framebuffer_sample_image_        = {};
	VkImageView    framebuffer_sample_image_view_   = {};
	VkDeviceMemory framebuffer_sample_image_memory_ = {};

	VkImage        depth_stencil_image_      = {};
	VkImageView    depth_stencil_image_view_ = {};
	VkDeviceMemory depth_stencil_memory_     = {};

	VkSurfaceCapabilitiesKHR surface_capabilities_ = {};

	BatchRender batch_render_ = {};
};

#endif //VKAPP_H
