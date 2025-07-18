//
// Created by apant on 05/07/2025.
//

#include "Include/VkApp.h"

#include "../FileSystem.h"
#include "Include/Graphics.h"

#define VOLK_IMPLEMENTATION
#include <volk/volk.h>

#include <cassert>
#include <sstream>
#include <SDL2/SDL_vulkan.h>

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
	allocator_ = static_cast<VkAllocationCallbacks>(Allocator());

	Graphics::CreateInstance(
		{"VK_LAYER_KHRONOS_validation"},
		{VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		 VK_KHR_SURFACE_EXTENSION_NAME,
		 VK_KHR_WIN32_SURFACE_EXTENSION_NAME},
		&allocator_,
		&instance_);

	volkLoadInstance(instance_);

	Graphics::CreateDebugMessenger(
		instance_,
		&allocator_,
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
	};

	// List the required gpu extensions
	const std::vector<const char*> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	Graphics::QueryGpu(
		instance_,
		gpu_required_features,
		device_extensions,
		&gpu_);

	uint32_t queue_family_index = 0;
	Graphics::QueryQueueFamily(
		gpu_,
		VK_QUEUE_GRAPHICS_BIT,
		true,
		0,
		nullptr,
		&queue_family_index);

	constexpr uint32_t queue_family_count = 1;
	Graphics::CreateDevice(
		gpu_,
		queue_family_count,
		&queue_family_index,
		device_extensions,
		&gpu_required_features,
		&allocator_,
		&device_);

	vkGetDeviceQueue(
		device_,
		queue_family_index,
		0,
		&queue_);

	VkSurfaceFormatKHR surface_format = {};
	Graphics::QuerySurfaceFormat(
		gpu_,
		surface_,
		{VK_FORMAT_R8G8B8A8_SRGB},
		&surface_format);

	Graphics::QuerySurfaceCapabilities(
		gpu_,
		surface_,
		&surface_capabilities_);

	Graphics::CreateSwapchain(
		device_,
		surface_,
		&surface_format,
		&surface_capabilities_,
		// At this point is VK_NULL_HANDLE
		swapchain_,
		&allocator_,
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

		Graphics::CreateBuffer(
			device_,
			gpu_,
			buffer_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&allocator_,
			&per_frame_data_buffers_[i],
			&per_frame_data_memories_[i]);
	}

	Graphics::QuerySwapchainImages(
		device_,
		swapchain_,
		&presentation_frames_.images[0]);

	for (uint32_t i = 0; i < surface_capabilities_.minImageCount; i++)
	{
		Graphics::CreateImageView(
			device_,
			presentation_frames_.images[i],
			VK_IMAGE_VIEW_TYPE_2D,
			surface_format.format,
			{
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY},
			&allocator_,
			&presentation_frames_.image_views[i]);
	}

	// Sample image resolver
	VkSampleCountFlagBits sample_counts = VK_SAMPLE_COUNT_1_BIT;
	Graphics::QuerySampleCounts(
		gpu_,
		&sample_counts);

	Graphics::CreateImage(
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
		&allocator_,
		&framebuffer_sample_image_,
		&framebuffer_sample_image_memory_);

	Graphics::CreateImageView(
		device_,
		framebuffer_sample_image_,
		VK_IMAGE_VIEW_TYPE_2D,
		surface_format.format,
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		},
		&allocator_,
		&framebuffer_sample_image_view_);

	Graphics::CreateCommandPool(
		device_,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		queue_family_index,
		&allocator_,
		&command_pool_);

	Graphics::CreateCommandBuffer(
		device_,
		command_pool_,
		&command_buffer_);

	// Synchronization

	Graphics::CreateSemaphore(
		device_,
		&allocator_,
		&image_available_semaphore_);

	Graphics::CreateSemaphore(
		device_,
		&allocator_,
		&render_finished_semaphore_);

	Graphics::CreateFence(
		device_,
		&allocator_,
		&submit_finished_fence_);

	const Batch batch = {
		{{0.0f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}},
		{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}},
		{0, 1, 2}
	};

	const size_t position_buffer_size = sizeof(glm::vec3) * batch.position.size();
	Graphics::CreateBuffer(
		device_,
		gpu_,
		position_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&allocator_,
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

	const size_t color_buffer_size = sizeof(glm::vec4) * batch.color.size();
	Graphics::CreateBuffer(
		device_,
		gpu_,
		color_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&allocator_,
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
	Graphics::CreateBuffer(
		device_,
		gpu_,
		index_buffer_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&allocator_,
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
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	const VkSubpassDescription subpass = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments = nullptr,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
		.pResolveAttachments = &color_attachment_resolve_ref,
		.pDepthStencilAttachment = nullptr,
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

	constexpr uint32_t            attachment_desc_count                   = 2;
	const VkAttachmentDescription attachment_descs[attachment_desc_count] = {
		color_attachment,
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
		&allocator_,
		&render_pass_));

	for (size_t i = 0; i < surface_capabilities_.minImageCount; i++)
	{
		constexpr uint32_t attachments_count              = 2;
		const VkImageView  attachments[attachments_count] = {
			framebuffer_sample_image_view_, // Multisample
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
			&allocator_,
			&presentation_frames_.framebuffers[i]));
	}

	// Vulkan Pipeline

	auto vert_shader_code = FileSystem::ReadFile("../Resources/Shaders/vert.spv");
	auto frag_shader_code = FileSystem::ReadFile("../Resources/Shaders/frag.spv");

	VkShaderModule shader_modules[2] = {};
	Graphics::CreateShaderModule(
		device_,
		vert_shader_code,
		&allocator_,
		&shader_modules[0]);

	Graphics::CreateShaderModule(
		device_,
		frag_shader_code,
		&allocator_,
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
		}
	};

	const VkVertexInputAttributeDescription attribute_description[2] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0
		},
		{
			.location = 1,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0
		}
	};

	const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 2,
		.pVertexBindingDescriptions = &bind_descs[0],
		.vertexAttributeDescriptionCount = 2,
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
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
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
		&allocator_,
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
		&allocator_,
		&pipeline_layout_));

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
		.pDepthStencilState = nullptr,
		.pColorBlendState = &color_blend_info,
		.pDynamicState = &dynamic_state_create_info,
		.layout = pipeline_layout_,
		.renderPass = render_pass_,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	VK_CHECK(vkCreateGraphicsPipelines(
		device_,
		VK_NULL_HANDLE,
		1,
		&pipeline_info,
		&allocator_,
		&pipeline_));

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
		&allocator_,
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

	vkDestroyShaderModule(device_, shader_modules[0], &allocator_);
	vkDestroyShaderModule(device_, shader_modules[1], &allocator_);
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
	glm::vec3       camera_pos        = {0.0f, 0.0f, 2.0f};
	glm::vec3       camera_pos_new    = {0.0f, 0.0f, 2.0f};
	glm::vec3       camera_front      = {0.0f, 0.0f, -1.0f};
	glm::vec3       camera_up         = {0.0f, 1.0f, 0.0f};
	constexpr float camera_move_speed = 1.639f;

	constexpr float p         = 1.0f / 100.0f;
	constexpr float t         = 0.369f;
	const float     half_time = -t / glm::log2(p);

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

		camera_pos = glm::mix(camera_pos, camera_pos_new, camera_lerp_alpha);

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

			glm::perspective(
				glm::radians(45.0f),
				static_cast<float>(surface_capabilities_.currentExtent.width) /
				static_cast<float>(surface_capabilities_.currentExtent.height),
				0.1f,
				10.0f),
		};

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

		constexpr VkClearValue clear_value = {
			.color = {
				.float32 = {0.0f, 0.0f, 0.0f, 1.0f}
			},
			// .depthStencil = {0.0f, 0}
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
			.clearValueCount = 1,
			.pClearValues = &clear_value,
		};

		vkCmdBeginRenderPass(
			command_buffer_,
			&render_pass_begin_info,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(
			command_buffer_,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_);

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
		};

		const VkDeviceSize offsets[] = {
			0,
			0
		};

		vkCmdBindVertexBuffers(
			command_buffer_,
			0,
			2,
			&binds_buffer[0],
			&offsets[0]);

		vkCmdBindIndexBuffer(
			command_buffer_,
			batch_render_.index_buffer,
			0,
			VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(
			command_buffer_,
			3,
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

	vkDestroySwapchainKHR(device_, swapchain_, &allocator_);
	vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
	vkDestroyCommandPool(device_, command_pool_, &allocator_);
	vkDestroyDevice(device_, &allocator_);
	vkDestroyInstance(instance_, &allocator_);

	SDL_DestroyWindow(window_);
	SDL_Quit();
}