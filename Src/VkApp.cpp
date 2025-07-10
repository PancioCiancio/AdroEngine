//
// Created by apant on 05/07/2025.
//

#include "VkApp.h"

#include "FileSystem.h"
#include "Graphics.h"

#include <cassert>
#include <iostream>
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


VkResult VkApp::Init()
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
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

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

	VkSurfaceCapabilitiesKHR surface_capabilities = {};
	Graphics::QuerySurfaceCapabilities(
		gpu_,
		surface_,
		&surface_capabilities);

	Graphics::CreateSwapchain(
		device_,
		surface_,
		&surface_format,
		&surface_capabilities,
		// At this point is VK_NULL_HANDLE
		swapchain_,
		&allocator_,
		&swapchain_);

	swapchain_images_.resize(surface_capabilities.minImageCount);
	Graphics::QuerySwapchainImages(
		device_,
		swapchain_,
		&swapchain_images_[0]);

	swapchain_image_views_.resize(surface_capabilities.minImageCount);
	for (uint32_t i = 0; i < surface_capabilities.minImageCount; i++)
	{
		Graphics::CreateImageView(
			device_,
			swapchain_images_[i],
			VK_IMAGE_VIEW_TYPE_2D,
			surface_format.format,
			{
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY},
			&allocator_,
			&swapchain_image_views_[i]);
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
			surface_capabilities.currentExtent.width,
			surface_capabilities.currentExtent.height,
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

	// Render Pass

	const VkAttachmentDescription color_attachment = {
		.flags = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.format = surface_format.format,
		.samples = sample_counts,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	const VkAttachmentReference color_attachment_ref = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription color_attachment_resolve = {
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

	VkAttachmentReference color_attachment_resolve_ref = {
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

	const VkSubpassDependency dependency = {
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

	framebuffers_.resize(surface_capabilities.minImageCount);

	for (size_t i = 0; i < surface_capabilities.minImageCount; i++)
	{
		constexpr uint32_t attachments_count              = 2;
		const VkImageView  attachments[attachments_count] = {
			framebuffer_sample_image_view_, // Multisample
			swapchain_image_views_[i], // Multisample resolver to 1 sample.
		};

		VkFramebufferCreateInfo framebuffer_info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderPass = render_pass_,
			.attachmentCount = attachments_count,
			.pAttachments = &attachments[0],
			.width = surface_capabilities.currentExtent.width,
			.height = surface_capabilities.currentExtent.height,
			.layers = 1,
		};

		VK_CHECK(vkCreateFramebuffer(
			device_,
			&framebuffer_info,
			&allocator_,
			&framebuffers_[i]));
	}

	// Vulkan Pipeline

	auto vert_shader_code = FileSystem::ReadFile("Resources/Shaders/vert.spv");
	auto frag_shader_code = FileSystem::ReadFile("Resources/Shaders/frag.spv");

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

	constexpr VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr
	};

	constexpr VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	constexpr VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = 640.0f,
		.height = 480.0f,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	constexpr VkRect2D scissor = {
		.offset = {0, 0},
		.extent = {640, 480},
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

	constexpr VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 0,
		.pSetLayouts = nullptr,
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

	vkDestroyShaderModule(device_, shader_modules[0], &allocator_);
	vkDestroyShaderModule(device_, shader_modules[1], &allocator_);

	return VK_SUCCESS;
}

// Timing variables
Uint64 NOW       = SDL_GetPerformanceCounter();
Uint64 LAST      = 0;
double deltaTime = 0;

// Target frame rate (optional)
constexpr int    targetFPS       = 60;
constexpr double targetFrameTime = 1000.0 / targetFPS; // in milliseconds

VkResult VkApp::Update()
{
	bool stillRunning = true;
	while (stillRunning)
	{
		LAST = NOW;
		NOW  = SDL_GetPerformanceCounter();

		// Calculate delta time in seconds
		deltaTime = static_cast<double>((NOW - LAST) * 1000) / static_cast<double>(SDL_GetPerformanceFrequency());

		SDL_Event event;
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
			.framebuffer = framebuffers_[next_image],
			.renderArea = {
				.offset = {0, 0},
				.extent = {640, 480},
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

		constexpr VkViewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = 640.0f,
			.height = 480.0f,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};

		vkCmdSetViewport(
			command_buffer_,
			0,
			1,
			&viewport);

		constexpr VkRect2D scissor = {
			.offset = {0, 0},
			.extent = {640, 480},
		};

		vkCmdSetScissor(
			command_buffer_,
			0,
			1,
			&scissor);

		vkCmdDraw(
			command_buffer_,
			3,
			1,
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
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.swapchainCount = 1,
			.pSwapchains = &swapchain_,
			.pImageIndices = &next_image,
			.pResults = &result,
		};

		VK_CHECK(vkQueuePresentKHR(queue_, &present_info));

		// --- Your game update & render logic here ---
		// Example: updateGame(deltaTime); render();

		// Frame limiting: Sleep if frame is faster than target frame time
		if (deltaTime < targetFrameTime)
		{
			SDL_Delay(static_cast<Uint32>(targetFrameTime - deltaTime));
		}
	}

	return VK_SUCCESS;
}

VkResult VkApp::TearDown() const
{
	VK_CHECK(vkDeviceWaitIdle(device_));

	vkDestroySwapchainKHR(device_, swapchain_, &allocator_);
	vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
	vkDestroyCommandPool(device_, command_pool_, &allocator_);
	vkDestroyDevice(device_, &allocator_);
	vkDestroyInstance(instance_, &allocator_);

	SDL_DestroyWindow(window_);
	SDL_Quit();

	return VK_SUCCESS;
}