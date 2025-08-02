//
// Created by apant on 02/08/2025.
//

#include "run.h"
#include "common.h"

#include <SDL2/SDL_vulkan.h>

namespace Renderer
{
void Renderer::init()
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

	init_instance();
	init_surface();
	init_device();
	init_swapchain();
}

void Renderer::init_instance()
{
	VK_CHECK(volkInitialize());

	// @todo:	calculate the layers count from the array.
	const char* requested_layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	// @todo:	calculate the extension count from the array.
	const char* requested_extensions[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	vk_create_instance(
		1,
		requested_layers,
		3,
		requested_extensions,
		nullptr,
		&instance_);

	volkLoadInstance(instance_);

	vk_create_debug_messenger(
		instance_,
		nullptrm
		& debug_messenger_);
}

void Renderer::init_surface()
{
	VK_CHECK(SDL_Vulkan_CreateSurface(
		         window_,
		         instance_,
		         &surface_)
		         ? VK_SUCCESS
		         : VK_ERROR_INITIALIZATION_FAILED);
}

void Renderer::init_device()
{
	const VkPhysicalDeviceFeatures gpu_required_features = {
		.geometryShader = VK_TRUE,
		.tessellationShader = VK_TRUE,
		.multiDrawIndirect = VK_TRUE,
		.fillModeNonSolid = VK_TRUE,
	};

	// @todo:	calculate the device extension count from the array.
	const char* device_extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk_query_gpu(
		instance_,
		gpu_required_features,
		1,
		device_extensions,
		&gpu_);

	vk_query_queue_family(
		gpu_,
		VK_QUEUE_GRAPHICS_BIT,
		true,
		0,
		nullptr,
		&queue_family_idx_);

	constexpr uint32_t queue_family_count = 1;
	vk_create_device(
		gpu_,
		queue_family_count,
		&queue_family_idx_,
		requested_device_ext_count,
		device_extensions,
		&gpu_required_features,
		nullptr,
		&device_);

	volkLoadDevice(device_);

	vkGetDeviceQueue(
		device_,
		queue_family_idx_,
		0,
		&queue_);
}

void Renderer::init_swapchain()
{

	VkFormat required_surface_formats[1] = {
		VK_FORMAT_R8G8B8A8_SRGB,
	};

	VkSurfaceFormatKHR surface_format = {};
	vk_query_surface_format(
		gpu_,
		surface_,
		required_surface_format_count,
		required_surface_formats,
		&surface_format);

	vk_query_surface_capabilities(
		gpu_,
		surface_,
		&surface_capabilities_);

	vk_create_swapchain(
		device_,
		surface_,
		&surface_format,
		&surface_capabilities_,
		// At this point is VK_NULL_HANDLE
		swapchain_,
		nullptr,
		&swapchain_);

	vk_query_swapchain_image_format(
		device_,
		swapchain_,
		&presentation_frames_.images[0]);

	// Create the view of the swapchain images
	for (uint32_t i = 0; i < surface_capabilities_.minImageCount; ++i)
	{
		vk_create_image_view(
			device,
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

	for (uint32_t i = 0; i < surface_capabilities_.minImageCount; ++i)
	{
		const VkSemaphoreCreateInfo semaphore_create_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};

		VK_CHECK(vkCreateSemaphore(
			device_,
			&semaphore_create_info,
			nullptr,
			presentation_frames_.render_finished_semaphores[i]));

		VK_CHECK(vkCreateSemaphore(
			device_,
			&semaphore_create_info,
			nullptr,
			presentation_frames_.image_available_semaphores[i]));

		VkFenceCreateInfo fence_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		VK_CHECK(vkCreateFence(
			device,
			&fence_info,
			nullptr,
			presentation_frames_.render_finished_fences[i]));
	}
}

void Renderer::init_other_images()
{
	VkSampleCountFlagBits sample_counts = VK_SAMPLE_COUNT_1_BIT;
	vk_query_sample_counts(
		gpu_,
		&sample_counts);

	vk_create_image(
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

	vk_create_image_view(
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

	vk_query_supported_format(
		gpu_,
		4,
		&depth_stencil_format_requested[0],
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
		&depth_stencil_format);

	vk_create_image(
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

	vk_create_image_view(
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
}

void Renderer::init_command()
{
	const VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = flags,
		.queueFamilyIndex = queue_family_idx_,
	};

	VK_CHECK(vkCreateCommandPool(
		device,
		&command_pool_create_info,
		nullptr,
		&command_pool_));

	const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = command_pool_,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VK_CHECK(vkAllocateCommandBuffers(
		device,
		&command_buffer_allocate_info,
		&command_buffer_));
}

void Renderer::init_batch()
{
	Mesh::Load("../Resources/Meshes/lucy.obj", &batch_data_);
	std::vector<glm::vec4> default_colors(batch_data_.position.size(), glm::vec4(.5f, .5f, .5f, 1.0f));
	batch_data_.color = default_colors;

	const size_t position_buffer_size = sizeof(glm::vec3) * batch_data_.position.size();
	vk_create_buffer(
		device_,
		gpu_,
		position_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_.position_buffer,
		&batch_.position_mem);

	void* position_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_.position_mem,
		0,
		position_buffer_size,
		0,
		&position_data));

	memcpy(
		position_data,
		&batch_data_.position[0],
		position_buffer_size);

	vkUnmapMemory(
		device_,
		batch_.position_mem);

	const size_t normal_buffer_size = sizeof(glm::vec3) * batch_data_.normals.size();
	vk_create_buffer(
		device_,
		gpu_,
		normal_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_.normal_buffer,
		&batch_.normal_mem);

	void* normal_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_.normal_mem,
		0,
		normal_buffer_size,
		0,
		&normal_data));

	memcpy(
		normal_data,
		&batch_data_.normals[0],
		normal_buffer_size);

	vkUnmapMemory(
		device_,
		batch_.normal_mem);

	const size_t color_buffer_size = sizeof(glm::vec4) * batch.color.size();
	vk_create_buffer(
		device_,
		gpu_,
		color_buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_.color_buffer,
		&batch_.color_mem);

	void* color_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_.color_mem,
		0,
		color_buffer_size,
		0,
		&color_data));

	memcpy(
		color_data,
		&batch_data_.color[0],
		color_buffer_size);

	vkUnmapMemory(
		device_,
		batch_.color_mem);

	const size_t index_buffer_size = sizeof(uint32_t) * batch.indices.size();
	vk_create_buffer(
		device_,
		gpu_,
		index_buffer_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		nullptr,
		&batch_.index_buffer,
		&batch_.index_mem);

	void* index_data = nullptr;
	VK_CHECK(vkMapMemory(
		device_,
		batch_.index_mem,
		0,
		index_buffer_size,
		0,
		&index_data));

	memcpy(
		index_data,
		&batch_data_.indices[0],
		index_buffer_size);

	vkUnmapMemory(
		device_,
		batch_.index_mem);
}

void Renderer::init_render_pass()
{
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
}

void Renderer::init_pipeline()
{
	auto vert_shader_code = FileSystem::ReadFile("../Resources/Shaders/vert.spv");
	auto frag_shader_code = FileSystem::ReadFile("../Resources/Shaders/frag.spv");

	VkShaderModule shader_modules[2] = {};

	const VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = static_cast<uint32_t>(vert_shader_code.size()),
		.pCode = reinterpret_cast<const uint32_t*>(vert_shader_code.data()),
	};

	VK_CHECK(vkCreateShaderModule(
		device,
		&create_info,
		nullptr,
		&shader_modules[0]));

	const VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = static_cast<uint32_t>(frag_shader_code.size()),
		.pCode = reinterpret_cast<const uint32_t*>(frag_shader_code.data()),
	};

	VK_CHECK(vkCreateShaderModule(
		device,
		&create_info,
		nullptr,
		&shader_modules[1]));

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

	VkSampleCountFlagBits sample_counts = VK_SAMPLE_COUNT_1_BIT;
	vk_query_sample_counts(
		gpu_,
		&sample_counts);

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
}
} // Renderer