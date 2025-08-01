//
// Created by apant on 05/07/2025.
//

#include "Include/VkApp.h"

#include "../FileSystem.h"
#include "Include/Graphics.h"
#include "Mesh.h"
#include "vk_instance.h"
#include "vk_physical_device.h"
#include "vk_device.h"
#include "vk_debug_messenger.h"
#include "vk_swapchain.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_fence.h"
#include "vk_semaphore.h"
#include "vk_allocator.h"
#include "vk_utils.h"
#include "vk_image.h"
#include "vk_shader_module.h"
#include "vk_buffer.h"

#define VOLK_IMPLEMENTATION
#include <volk/volk.h>

#include <cassert>
#include <sstream>
#include <SDL2/SDL_vulkan.h>


void VkApp::Init()
{
	// Init the window class
	VK_CHECK((SDL_Init(SDL_INIT_VIDEO) == 0)
		? VK_SUCCESS
		: VK_ERROR_UNKNOWN);

	window_ = SDL_CreateWindow(
		"Adro Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_VULKAN);

	VK_CHECK(volkInitialize());

	// Create the custom allocator
	allocator_ = static_cast<VkAllocationCallbacks>(Gfx::Allocator());

	// @todo:	calculate the layers count from the array.
	constexpr uint32_t requested_layer_count                   = 1;
	const char*        requested_layers[requested_layer_count] = {
		"VK_LAYER_KHRONOS_validation"
	};

	// @todo:	calculate the extension count from the array.
	constexpr uint32_t requested_extension_count                       = 3;
	const char*        requested_extensions[requested_extension_count] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	Gfx::CreateInstance(
		requested_layer_count,
		requested_layers,
		requested_extension_count,
		requested_extensions,
		nullptr,
		&instance_);

	volkLoadInstance(instance_);

	Gfx::CreateDebugMessenger(
		instance_,
		nullptr,
		&debug_messenger_);

	// Create the surface
	VK_CHECK(SDL_Vulkan_CreateSurface(
			window_,
			instance_,
			&surface_)
		? VK_SUCCESS
		: VK_ERROR_UNKNOWN);

	// List the required gpu features
	const VkPhysicalDeviceFeatures gpu_required_features = {
		.geometryShader = VK_TRUE,
		.tessellationShader = VK_TRUE,
		.multiDrawIndirect = VK_TRUE,
		.fillModeNonSolid = VK_TRUE,
	};

	// @todo:	calculate the device extension count from the array.
	constexpr uint32_t requested_device_ext_count                    = 1;
	const char*        device_extensions[requested_device_ext_count] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	Gfx::QueryGpu(
		instance_,
		gpu_required_features,
		requested_device_ext_count,
		device_extensions,
		&gpu_);

	uint32_t queue_family_index = 0;
	Gfx::QueryQueueFamily(
		gpu_,
		VK_QUEUE_GRAPHICS_BIT,
		true,
		0,
		nullptr,
		&queue_family_index);

	constexpr uint32_t queue_family_count = 1;
	Gfx::CreateDevice(
		gpu_,
		queue_family_count,
		&queue_family_index,
		requested_device_ext_count,
		device_extensions,
		&gpu_required_features,
		nullptr,
		&device_);

	vkGetDeviceQueue(
		device_,
		queue_family_index,
		0,
		&queue_);

	constexpr uint32_t required_surface_format_count = 1;
	VkFormat           required_surface_formats[1]   = {
		VK_FORMAT_R8G8B8A8_SRGB,
	};

	VkSurfaceFormatKHR surface_format = {};
	Gfx::QuerySurfaceFormat(
		gpu_,
		surface_,
		required_surface_format_count,
		required_surface_formats,
		&surface_format);

	Gfx::QuerySurfaceCapabilities(
		gpu_,
		surface_,
		&surface_capabilities_);

	Gfx::CreateSwapchain(
		device_,
		surface_,
		&surface_format,
		&surface_capabilities_,
		// At this point is VK_NULL_HANDLE
		swapchain_,
		nullptr,
		&swapchain_);

	// Resize per-frame presentation
	presentation_frames_.images.resize(surface_capabilities_.minImageCount);
	presentation_frames_.image_views.resize(surface_capabilities_.minImageCount);
	presentation_frames_.framebuffers.resize(surface_capabilities_.minImageCount);

	// Resize per-frame data (Uniform Buffer)
	per_frame_data_buffers_.resize(surface_capabilities_.minImageCount);
	per_frame_data_mapped_.resize(surface_capabilities_.minImageCount);
	per_frame_data_memories_.resize(surface_capabilities_.minImageCount);

	for (uint32_t i = 0; i < surface_capabilities_.minImageCount; i++)
	{
		constexpr VkDeviceSize buffer_size = sizeof(Graphics::PerFrameData);

		Gfx::CreateBuffer(
			device_,
			gpu_,
			buffer_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			nullptr,
			&per_frame_data_buffers_[i],
			&per_frame_data_memories_[i]);
	}

	Gfx::QuerySwapchainImages(
		device_,
		swapchain_,
		&presentation_frames_.images[0]);

	for (uint32_t i = 0; i < surface_capabilities_.minImageCount; i++)
	{
		Gfx::CreateImageView(
			device_,
			presentation_frames_.images[i],
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_VIEW_TYPE_2D,
			surface_format.format,
			{
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY},
			nullptr,
			&presentation_frames_.image_views[i]);
	}

	// Sample image resolver
	VkSampleCountFlagBits sample_counts = VK_SAMPLE_COUNT_1_BIT;
	Gfx::QuerySampleCounts(
		gpu_,
		&sample_counts);

	Gfx::CreateImage(
		device_,
		gpu_,
		VK_IMAGE_TYPE_2D,
		surface_format.format,
		{
			surface_capabilities_.currentExtent.width,
			surface_capabilities_.currentExtent.height,
			1
		},
		sample_counts,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		nullptr,
		&framebuffer_sample_image_,
		&framebuffer_sample_image_memory_);

	Gfx::CreateImageView(
		device_,
		framebuffer_sample_image_,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_VIEW_TYPE_2D,
		surface_format.format,
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		},
		nullptr,
		&framebuffer_sample_image_view_);

	// Depth + Stencil

	constexpr VkFormat depth_stencil_format_requested[] = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
	};

	VkFormat depth_stencil_format = {};

	Gfx::QuerySupportedFormat(
		gpu_,
		4,
		&depth_stencil_format_requested[0],
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
		&depth_stencil_format);

	Gfx::CreateImage(
		device_,
		gpu_,
		VK_IMAGE_TYPE_2D,
		depth_stencil_format,
		{surface_capabilities_.currentExtent.width, surface_capabilities_.currentExtent.width, 1},
		sample_counts,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		nullptr,
		&depth_stencil_image_,
		&depth_stencil_memory_);

	Gfx::CreateImageView(
		device_,
		depth_stencil_image_,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_VIEW_TYPE_2D,
		depth_stencil_format,
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		},
		nullptr,
		&depth_stencil_image_view_);

	Gfx::CreateCommandPool(
		device_,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		queue_family_index,
		nullptr,
		&command_pool_);

	Gfx::CreateCommandBuffer(
		device_,
		command_pool_,
		&command_buffer_);

	// Synchronization

	Gfx::CreateSemaphore(
		device_,
		nullptr,
		&image_available_semaphore_);

	Gfx::CreateSemaphore(
		device_,
		nullptr,
		&render_finished_semaphore_);

	Gfx::CreateFence(
		device_,
		nullptr,
		&submit_finished_fence_);

	Batch batch = {};
	Mesh::Load("../Resources/Meshes/lucy.obj", &batch);
	std::vector<glm::vec4> default_colors(batch.position.size(), glm::vec4(.5f, .5f, .5f, 1.0f));
	batch.color = default_colors;

	const size_t position_buffer_size = sizeof(glm::vec3) * batch.position.size();
	Gfx::CreateBuffer(
		device_,
		gpu_,
		position_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_render_.position_buffer,
		&batch_render_.position_memory);

	void* position_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_render_.position_memory,
		0,
		position_buffer_size,
		0,
		&position_data));

	memcpy(
		position_data,
		&batch.position[0],
		position_buffer_size);

	vkUnmapMemory(
		device_,
		batch_render_.position_memory);

	const size_t normal_buffer_size = sizeof(glm::vec3) * batch.normals.size();
	Gfx::CreateBuffer(
		device_,
		gpu_,
		normal_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_render_.normal_buffer,
		&batch_render_.normal_memory);

	void* normal_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_render_.normal_memory,
		0,
		normal_buffer_size,
		0,
		&normal_data));

	memcpy(
		normal_data,
		&batch.normals[0],
		normal_buffer_size);

	vkUnmapMemory(
		device_,
		batch_render_.normal_memory);

	const size_t color_buffer_size = sizeof(glm::vec4) * batch.color.size();
	Gfx::CreateBuffer(
		device_,
		gpu_,
		color_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_render_.color_buffer,
		&batch_render_.color_memory);

	void* color_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_render_.color_memory,
		0,
		color_buffer_size,
		0,
		&color_data));

	memcpy(
		color_data,
		&batch.color[0],
		color_buffer_size);

	vkUnmapMemory(
		device_,
		batch_render_.color_memory);

	const size_t index_buffer_size = sizeof(uint32_t) * batch.indices.size();
	Gfx::CreateBuffer(
		device_,
		gpu_,
		index_buffer_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_render_.index_buffer,
		&batch_render_.index_memory);

	void* index_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_render_.index_memory,
		0,
		index_buffer_size,
		0,
		&index_data));

	memcpy(
		index_data,
		&batch.indices[0],
		index_buffer_size);

	vkUnmapMemory(
		device_,
		batch_render_.index_memory);

	// Render Pass

	// @todo:	Render pass is per-application implementation.
	//			We can provide the most common ones in another library that depends on Graphics.

	const VkAttachmentDescription color_attachment = {
		.flags = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.format = surface_format.format,
		.samples = sample_counts,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	constexpr VkAttachmentReference color_attachment_ref = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	const VkAttachmentDescription color_attachment_resolve = {
		.flags = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.format = surface_format.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	constexpr VkAttachmentReference color_attachment_resolve_ref = {
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	// Depth + stencil
	const VkAttachmentDescription depth_attachment = {
		.flags = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.format = depth_stencil_format,
		.samples = sample_counts,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	const VkAttachmentReference depth_attachment_reference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	const VkSubpassDescription subpass = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments = nullptr,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
		.pResolveAttachments = &color_attachment_resolve_ref,
		.pDepthStencilAttachment = &depth_attachment_reference,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = nullptr,
	};

	constexpr VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = 0,
	};

	constexpr uint32_t            attachment_desc_count                   = 3;
	const VkAttachmentDescription attachment_descs[attachment_desc_count] = {
		color_attachment,
		depth_attachment,
		color_attachment_resolve,
	};

	const VkRenderPassCreateInfo render_pass_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.attachmentCount = attachment_desc_count,
		.pAttachments = &attachment_descs[0],
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	VK_CHECK(vkCreateRenderPass(
		device_,
		&render_pass_create_info,
		nullptr,
		&render_pass_));

	for (size_t i = 0; i < surface_capabilities_.minImageCount; i++)
	{
		constexpr uint32_t attachments_count              = 3;
		const VkImageView  attachments[attachments_count] = {
			framebuffer_sample_image_view_, // Multisample
			depth_stencil_image_view_,
			presentation_frames_.image_views[i], // Multisample resolver to 1 sample.
		};

		VkFramebufferCreateInfo framebuffer_info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderPass = render_pass_,
			.attachmentCount = attachments_count,
			.pAttachments = &attachments[0],
			.width = surface_capabilities_.currentExtent.width,
			.height = surface_capabilities_.currentExtent.height,
			.layers = 1,
		};

		VK_CHECK(vkCreateFramebuffer(
			device_,
			&framebuffer_info,
			nullptr,
			&presentation_frames_.framebuffers[i]));
	}

	// @todo:	Pipelines are per-application specific as well.
	//			We should provide the most common ones in another library that depends on Graphics.

	auto vert_shader_code = FileSystem::ReadFile("../Resources/Shaders/vert.spv");
	auto frag_shader_code = FileSystem::ReadFile("../Resources/Shaders/frag.spv");

	VkShaderModule shader_modules[2] = {};
	Gfx::CreateShaderModule(
		device_,
		static_cast<uint32_t>(vert_shader_code.size()),
		vert_shader_code.data(),
		nullptr,
		&shader_modules[0]);

	Gfx::CreateShaderModule(
		device_,
		static_cast<uint32_t>(frag_shader_code.size()),
		frag_shader_code.data(),
		nullptr,
		&shader_modules[1]);

	const VkPipelineShaderStageCreateInfo vert_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = shader_modules[0],
		.pName = "main",
		.pSpecializationInfo = nullptr,
	};

	const VkPipelineShaderStageCreateInfo frag_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = shader_modules[1],
		.pName = "main",
		.pSpecializationInfo = nullptr,
	};

	const VkPipelineShaderStageCreateInfo shaderStages[] = {vert_shader_stage_create_info,
	                                                        frag_shader_stage_create_info};

	const std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	const VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data(),
	};

	const VkVertexInputBindingDescription bind_descs[] = {
		// position
		{
			.binding = 0,
			.stride = sizeof(glm::vec3),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		},
		// color.
		{
			.binding = 1,
			.stride = sizeof(glm::vec4),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		},
		// normal.
		{
			.binding = 2,
			.stride = sizeof(glm::vec3),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		}
	};

	const VkVertexInputAttributeDescription attribute_description[3] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0
		},
		{
			.location = 1,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = 0
		},
		{
			.location = 2,
			.binding = 2,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0
		}
	};

	const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 3,
		.pVertexBindingDescriptions = &bind_descs[0],
		.vertexAttributeDescriptionCount = 3,
		.pVertexAttributeDescriptions = &attribute_description[0],
	};

	constexpr VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	const VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(surface_capabilities_.currentExtent.width),
		.height = static_cast<float>(surface_capabilities_.currentExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const VkRect2D scissor = {
		.offset = {0, 0},
		.extent = surface_capabilities_.currentExtent,
	};

	// ReSharper disable once CppVariableCanBeMadeConstexpr
	const VkPipelineViewportStateCreateInfo viewport_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	constexpr VkPipelineRasterizationStateCreateInfo rasterization_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};

	const VkPipelineMultisampleStateCreateInfo multisample_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = sample_counts,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	const VkPipelineColorBlendAttachmentState color_blend_attachment = {
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		                  VK_COLOR_COMPONENT_A_BIT,
	};

	// ReSharper disable once CppVariableCanBeMadeConstexpr
	const VkPipelineColorBlendStateCreateInfo color_blend_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
		.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
	};

	const VkDescriptorSetLayoutBinding u_buffer_set_binding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr,
	};

	const VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &u_buffer_set_binding
	};

	VK_CHECK(vkCreateDescriptorSetLayout(
		device_,
		&layout_info,
		nullptr,
		&descriptor_set_layout_));

	const VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_set_layout_,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr,
	};

	VK_CHECK(vkCreatePipelineLayout(
		device_,
		&pipeline_layout_info,
		nullptr,
		&pipeline_layout_));

	// depth + stencil
	const VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {},
		.back = {},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};

	const VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly_info,
		.pTessellationState = nullptr,
		.pViewportState = &viewport_info,
		.pRasterizationState = &rasterization_info,
		.pMultisampleState = &multisample_info,
		.pDepthStencilState = &depth_stencil_state_create_info,
		.pColorBlendState = &color_blend_info,
		.pDynamicState = &dynamic_state_create_info,
		.layout = pipeline_layout_,
		.renderPass = render_pass_,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	constexpr VkPipelineRasterizationStateCreateInfo rasterization_info_wireframe = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_LINE,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};

	const VkGraphicsPipelineCreateInfo pipeline_info_wireframe = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly_info,
		.pTessellationState = nullptr,
		.pViewportState = &viewport_info,
		.pRasterizationState = &rasterization_info_wireframe,
		.pMultisampleState = &multisample_info,
		.pDepthStencilState = &depth_stencil_state_create_info,
		.pColorBlendState = &color_blend_info,
		.pDynamicState = &dynamic_state_create_info,
		.layout = pipeline_layout_,
		.renderPass = render_pass_,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	VkGraphicsPipelineCreateInfo pipeline_infos[] = {
		pipeline_info,
		pipeline_info_wireframe,
	};

	VkPipeline pipelines[] = {
		pipeline_,
		pipeline_wireframe_,
	};

	VK_CHECK(vkCreateGraphicsPipelines(
		device_,
		VK_NULL_HANDLE,
		2,
		&pipeline_infos[0],
		nullptr,
		&pipelines[0]));

	pipeline_           = pipelines[0];
	pipeline_wireframe_ = pipelines[1];

	const VkDescriptorPoolSize pool_size = {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = surface_capabilities_.minImageCount
	};

	VkDescriptorPoolCreateInfo pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = surface_capabilities_.minImageCount,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
	};

	VK_CHECK(vkCreateDescriptorPool(
		device_,
		&pool_create_info,
		nullptr,
		&descriptor_pool_));

	std::vector<VkDescriptorSetLayout> layouts(
		surface_capabilities_.minImageCount,
		descriptor_set_layout_);

	VkDescriptorSetAllocateInfo set_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptor_pool_,
		.descriptorSetCount = surface_capabilities_.minImageCount,
		.pSetLayouts = &layouts[0],
	};

	descriptor_sets_.resize(surface_capabilities_.minImageCount);

	VK_CHECK(vkAllocateDescriptorSets(
		device_,
		&set_allocate_info,
		&descriptor_sets_[0]));

	for (size_t i = 0; i < surface_capabilities_.minImageCount; i++)
	{
		VkDescriptorBufferInfo buffer_info = {
			.buffer = per_frame_data_buffers_[i],
			.offset = 0,
			.range = sizeof(Graphics::PerFrameData),
		};

		VkWriteDescriptorSet descriptor_set = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptor_sets_[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &buffer_info
		};

		vkUpdateDescriptorSets(
			device_,
			1,
			&descriptor_set,
			0,
			nullptr);
	}

	vkDestroyShaderModule(device_, shader_modules[0], nullptr);
	vkDestroyShaderModule(device_, shader_modules[1], nullptr);

	// Transition depth + stencil image layout.

	const VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(
		command_buffer_,
		&command_buffer_begin_info);

	const VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.image = depth_stencil_image_,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	const VkPipelineStageFlags source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	const VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	vkCmdPipelineBarrier(
		command_buffer_,
		source_stage,
		destination_stage,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier);

	vkEndCommandBuffer(command_buffer_);

	const VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer_
	};

	vkQueueSubmit(
		queue_,
		1,
		&submit_info,
		VK_NULL_HANDLE);

	vkQueueWaitIdle(queue_);
}

// Timing variables
Uint64 NOW       = SDL_GetPerformanceCounter();
Uint64 LAST      = 0;
double deltaTime = 0;

// Target frame rate (optional)
constexpr int    targetFPS       = 60;
constexpr double targetFrameTime = 1000.0 / targetFPS; // in milliseconds

void VkApp::Update()
{
	// Camera data
	glm::vec3       camera_pos        = {0.0f, 140.0f, -1900.0f};
	glm::vec3       camera_pos_new    = {0.0f, 140.0f, -1900.0f};
	glm::vec3       camera_front      = {0.0f, 0.0f, 1.0f};
	glm::vec3       camera_up         = {0.0f, 1.0f, 0.0f};
	constexpr float camera_move_speed = 200.639f;

	constexpr float p         = 1.0f / 100.0f;
	constexpr float t         = 0.396f;
	const float     half_time = -t / glm::log2(p);

	VkPipeline chosen_pipeline = pipeline_;

	bool stillRunning = true;
	while (stillRunning)
	{
		LAST = NOW;
		NOW  = SDL_GetPerformanceCounter();

		// Calculate delta time in seconds
		deltaTime = static_cast<double>(NOW - LAST) / static_cast<double>(SDL_GetPerformanceFrequency());

		const float camera_lerp_alpha = 1.0f - glm::pow(2.0f, -static_cast<float>(deltaTime) / half_time);

		SDL_Event event;

		// Input state

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				stillRunning = false;
				break;

			default:
				// Do nothing.
				break;
			}
		}

		const Uint8* key_states = SDL_GetKeyboardState(nullptr);

		if (key_states[SDL_SCANCODE_W])
		{
			camera_pos_new += camera_move_speed * static_cast<float>(deltaTime) * camera_front;
		}

		if (key_states[SDL_SCANCODE_S])
		{
			camera_pos_new -= camera_move_speed * static_cast<float>(deltaTime) * camera_front;
		}

		if (key_states[SDL_SCANCODE_A])
		{
			camera_pos_new -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_move_speed *
				static_cast<
					float>(deltaTime);
		}

		if (key_states[SDL_SCANCODE_D])
		{
			camera_pos_new += glm::normalize(glm::cross(camera_front, camera_up)) * camera_move_speed *
				static_cast<
					float>(deltaTime);
		}

		if (key_states[SDL_SCANCODE_Q])
		{
			chosen_pipeline = pipeline_wireframe_;
		}
		else if (key_states[SDL_SCANCODE_E])
		{
			chosen_pipeline = pipeline_;
		}

		camera_pos = glm::mix(camera_pos, camera_pos_new, camera_lerp_alpha);

		// @todo:	Since render pass and pipelines are per-application specific,
		//			Also the loop should be. We can provide an example code and let the final application implement it.

		vkWaitForFences(
			device_,
			1,
			&submit_finished_fence_,
			VK_TRUE,
			UINT64_MAX);

		vkResetFences(
			device_,
			1,
			&submit_finished_fence_);

		uint32_t next_image = 0u;
		VK_CHECK(vkAcquireNextImageKHR(
			device_,
			swapchain_,
			UINT64_MAX,
			image_available_semaphore_,
			VK_NULL_HANDLE,
			&next_image));

		Graphics::PerFrameData u_buffer = {
			glm::lookAt(
				camera_pos,
				camera_pos + camera_front,
				camera_up),

			glm::perspectiveRH_ZO(
				glm::radians(45.0f),
				static_cast<float>(surface_capabilities_.currentExtent.width) /
				static_cast<float>(surface_capabilities_.currentExtent.height),
				0.1f,
				10000.0f),
		};

		// Flip vulkan Y-axis
		u_buffer.projection[1][1] *= -1;

		VK_CHECK(vkMapMemory(
			device_,
			per_frame_data_memories_[next_image],
			0,
			sizeof(u_buffer),
			0,
			&per_frame_data_mapped_[next_image]));

		constexpr size_t u_buffer_size = sizeof(Graphics::PerFrameData);

		memcpy(per_frame_data_mapped_[next_image], &u_buffer, u_buffer_size);

		vkUnmapMemory(
			device_,
			per_frame_data_memories_[next_image]);

		const VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};

		VK_CHECK(vkResetCommandBuffer(
			command_buffer_,
			0));

		VK_CHECK(vkBeginCommandBuffer(
			command_buffer_,
			&begin_info));

		vkCmdBindDescriptorSets(
			command_buffer_,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_layout_,
			0,
			1,
			&descriptor_sets_[next_image],
			0,
			nullptr);

		constexpr VkClearValue clear_value[2] = {
			{
				.color = {
					.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
			},
			{
				.depthStencil = {1.0f, 0},
			}
		};

		VkRenderPassBeginInfo render_pass_begin_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = render_pass_,
			.framebuffer = presentation_frames_.framebuffers[next_image],
			.renderArea = {
				.offset = {0, 0},
				.extent = surface_capabilities_.currentExtent,
			},
			.clearValueCount = 2,
			.pClearValues = &clear_value[0],
		};

		vkCmdBeginRenderPass(
			command_buffer_,
			&render_pass_begin_info,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(
			command_buffer_,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			chosen_pipeline);

		const VkViewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(surface_capabilities_.currentExtent.width),
			.height = static_cast<float>(surface_capabilities_.currentExtent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};

		vkCmdSetViewport(
			command_buffer_,
			0,
			1,
			&viewport);

		const VkRect2D scissor = {
			.offset = {0, 0},
			.extent = surface_capabilities_.currentExtent,
		};

		vkCmdSetScissor(
			command_buffer_,
			0,
			1,
			&scissor);

		const VkBuffer binds_buffer[] = {
			batch_render_.position_buffer,
			batch_render_.color_buffer,
			batch_render_.normal_buffer,
		};

		const VkDeviceSize offsets[] = {
			0,
			0,
			0
		};

		vkCmdBindVertexBuffers(
			command_buffer_,
			0,
			3,
			&binds_buffer[0],
			&offsets[0]);

		vkCmdBindIndexBuffer(
			command_buffer_,
			batch_render_.index_buffer,
			0,
			VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(
			command_buffer_,
			299910,
			1,
			0,
			0,
			0);

		vkCmdEndRenderPass(
			command_buffer_);

		VK_CHECK(vkEndCommandBuffer(
			command_buffer_));

		VkSemaphore wait_semaphores[] = {
			image_available_semaphore_};
		VkPipelineStageFlags wait_stages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		VkSemaphore signal_semaphores[] = {
			render_finished_semaphore_};

		VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = wait_semaphores,
			.pWaitDstStageMask = wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &command_buffer_,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signal_semaphores,
		};

		VK_CHECK(vkQueueSubmit(
			queue_,
			1,
			&submit_info,
			submit_finished_fence_));

		VkResult               result       = {};
		const VkPresentInfoKHR present_info = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &render_finished_semaphore_,
			.swapchainCount = 1,
			.pSwapchains = &swapchain_,
			.pImageIndices = &next_image,
			.pResults = &result,
		};

		// @todo: cannot present the image if the window is minimized.
		VK_CHECK(vkQueuePresentKHR(queue_, &present_info));

		// --- Your game update & render logic here ---
		// Example: updateGame(deltaTime); render();

		// Frame limiting: Sleep if frame is faster than target frame time
		if (deltaTime < targetFrameTime)
		{
			SDL_Delay(static_cast<Uint32>(targetFrameTime - deltaTime));
		}
	}
}

void VkApp::TearDown() const
{
	VK_CHECK(vkDeviceWaitIdle(device_));

	vkDestroySwapchainKHR(device_, swapchain_, nullptr);
	vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
	vkDestroyCommandPool(device_, command_pool_, nullptr);
	vkDestroyDevice(device_, nullptr);
	vkDestroyInstance(instance_, nullptr);

	SDL_DestroyWindow(window_);
	SDL_Quit();
}