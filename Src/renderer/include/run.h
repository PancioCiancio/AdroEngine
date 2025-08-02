//
// Created by apant on 02/08/2025.
//

#ifndef RUN_H
#define RUN_H

#include <volk/volk.h>
#include <SDL2/SDL.h>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace Renderer
{
/// Implement application specific render logic (e.g.
/// batch, pipeline, framebuffers, loop, ...)

/// Groups all vertex info of all geometries that fit one draw call.
struct BatchCpu
{
	std::vector<glm::vec3> position;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> color;
	std::vector<uint32_t>  indices;
};

/// Groups all gpu buffers and gpu memories that fit one draw call command.
/// It reflects the BatchCpu data.
struct BatchGpu
{
	VkBuffer       position_buffer = {};
	VKDeviceMemory position_mem    = {};

	VkBuffer       normal_buffer = {};
	VKDeviceMemory normal_mem    = {};

	VkBuffer       color_buffer = {};
	VKDeviceMemory color_mem    = {};

	VkBuffer       index_buffer = {};
	VKDeviceMemory index_mem    = {};
};

/// Uniform buffer
struct PerFrameDataCpu
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};

template <size_t N>
struct PerFrameDataGpu
{
	VkBuffer        buffers[N]         = {};
	VkDeviceMemory  memory[N]          = {};
	void*           data_mapped[N]     = {};
	VkDescriptorSet descriptor_sets[N] = {};
};

/// Presentation frame-in-flight.
template <size_t N>
struct PerFramePresentationGpu
{
	VkImage       images[N]       = {};
	VkImageView   image_views[N]  = {};
	VkFramebuffer framebuffers[N] = {};

	VkSemaphore image_available_semaphores[N] = {};
	VkSemaphore render_finished_semaphores[N] = {};
	VkFence     submit_finished_fences[N]     = {};
};

class Renderer
{
public:
	/// Init all renderer resources
	void init();

	/// Issue renderer commands and draw on the screen
	void Update(double delta_time);

	/// De-init all renderer resources in order
	void Teardown() const;

private:
	void init_instance();

	void init_surface();

	void init_device();

	void init_swapchain();

	/// Multi-sample image resolver, depth-stencil image
	void init_other_images();

	void init_command();

	void init_batch();

	void init_renderpass();

	void init_pipeline();

private:
	// ======================
	// Logic
	// ======================
	VkInstance                    instance_             = {};
	VkDebugUtilsMessengerEXT      debug_messenger_      = {};
	std::vector<VkPhysicalDevice> gpus_                 = {};
	VkPhysicalDevice              gpu_                  = {};
	VkDevice                      device_               = {};
	VkQueue                       queue_                = {};
	uint32_t                      queue_family_idx_     = {};
	VkCommandPool                 command_pool_         = {};
	VkCommandBuffer               command_buffer_       = {};
	VkSurfaceCapabilitiesKHR      surface_capabilities_ = {};

	// ======================
	// Presentation
	// ======================
	SDL_Window*                window_              = {};
	VkSurfaceKHR               surface_             = {};
	VkSwapchainKHR             swapchain_           = {};
	PerFramePresentationGpu<2> presentation_frames_ = {};

	VkImage        framebuffer_sample_image_        = {};
	VkImageView    framebuffer_sample_image_view_   = {};
	VkDeviceMemory framebuffer_sample_image_memory_ = {};

	VkImage        depth_stencil_image_      = {};
	VkImageView    depth_stencil_image_view_ = {};
	VkDeviceMemory depth_stencil_memory_     = {};

	// ======================
	// Pipeline
	// ======================
	PerFrameDataGpu<2>    uniform_buffer_frames_ = {};
	VkRenderPass          render_pass_           = {};
	VkPipelineLayout      pipeline_layout_       = {};
	VkPipeline            pipeline_              = {};
	VkPipeline            pipeline_wireframe_    = {};
	VkDescriptorSetLayout descriptor_set_layout_ = {};
	VkDescriptorPool      descriptor_pool_       = {};

	// ======================
	// Batching
	// ======================
	BatchCpu batch_data_ = {};
	BatchGpu batch_      = {};
};
}

#endif //RUN_H
