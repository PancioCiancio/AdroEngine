//
// Created by apant on 07/07/2025.
//

#include "Graphics.h"

#include <cassert>
#include <stdexcept>
#include <cstring>

void Graphics::CreateInstance(
	const std::vector<const char*>& layers_requested,
	const std::vector<const char*>& extensions_requested,
	VkAllocationCallbacks*          p_allocator,
	VkInstance*                     p_instance)
{
	uint32_t layer_count = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(
		&layer_count,
		nullptr));

	std::vector<VkLayerProperties> layer_propertieses(layer_count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(
		&layer_count,
		&layer_propertieses[0]));

	uint32_t extension_count = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extension_count,
		nullptr));

	std::vector<VkExtensionProperties> extension_propertieses(extension_count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extension_count,
		&extension_propertieses[0]));

	// @todo	check for layer and extensions required.

	const VkDebugUtilsMessengerCreateInfoEXT debug_create_info_ext = GetDebugMessengerCreateInfo();

	const VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Adro",
		.applicationVersion = 1,
		.apiVersion = VK_MAKE_VERSION(1, 0, 0)
	};

	const VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &debug_create_info_ext,
		.pApplicationInfo = &app_info,
		.enabledExtensionCount = static_cast<uint32_t>(extensions_requested.size()),
		.ppEnabledExtensionNames = &extensions_requested[0],
	};

	VK_CHECK(vkCreateInstance(
		&create_info,
		p_allocator,
		p_instance));
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT             messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*                                       pUserData)
{
	const char* severity = "";
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		severity = "[VERBOSE]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		severity = "[INFO]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		severity = "[WARNING]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		severity = "[ERROR]";
		break;
	default:
		severity = "";
	}

	// @todo	cover the message type as well....

	std::printf("[VK] %s %s\n", severity, pCallbackData->pMessage);

	return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT Graphics::GetDebugMessengerCreateInfo()
{
	return {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = nullptr,
		.flags = 0,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback,
		.pUserData = nullptr,
	};
}

void Graphics::CreateDebugMessenger(
	VkInstance                instance,
	VkAllocationCallbacks*    p_allocator,
	VkDebugUtilsMessengerEXT* p_debuge_messenger_ext)
{
	const VkDebugUtilsMessengerCreateInfoEXT debug_create_info_ext = GetDebugMessengerCreateInfo();

	VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance,
		&debug_create_info_ext,
		p_allocator,
		p_debuge_messenger_ext));
}

void Graphics::SelectGpu(
	VkInstance                      instance,
	VkPhysicalDeviceFeatures        fts_requested,
	const std::vector<const char*>& extensions_requested,
	VkPhysicalDevice*               p_gpu)
{
	uint32_t gpu_count = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(
		instance,
		&gpu_count,
		nullptr));

	std::vector<VkPhysicalDevice> gpus(gpu_count);
	VK_CHECK(vkEnumeratePhysicalDevices(
		instance,
		&gpu_count,
		&gpus[0]));

	// Helper lambda to compare feature requested to supported.
	auto fn_ext = [](const VkBool32 requested, const VkBool32 supported) -> VkResult
	{
		return (requested == VK_TRUE && supported == VK_FALSE)
			       ? VK_ERROR_FEATURE_NOT_PRESENT
			       : VK_SUCCESS;
	};

	// Alghoritm
	std::vector<int32_t> gpus_rating(gpu_count);

	for (uint32_t i = 0; i < gpu_count; i++)
	{
		VkPhysicalDevice examine_gpu = gpus[i];

		VkPhysicalDeviceProperties gpu_properties = {};
		vkGetPhysicalDeviceProperties(examine_gpu, &gpu_properties);

		uint32_t gpu_extension_count = 0;
		VK_CHECK(vkEnumerateDeviceExtensionProperties(
			examine_gpu,
			nullptr,
			&gpu_extension_count,
			nullptr));

		std::vector<VkExtensionProperties> gpu_extension_propertieses(gpu_extension_count);
		VK_CHECK(vkEnumerateDeviceExtensionProperties(
			examine_gpu,
			nullptr,
			&gpu_extension_count,
			&gpu_extension_propertieses[0]));

		// Check extensions.
		bool all_ext_supported = false;

		for (auto extension : extensions_requested)
		{
			bool ext_supported = false;

			for (uint32_t k = 0; k < gpu_extension_count && !ext_supported; k++)
			{
				if (std::strcmp(extension, gpu_extension_propertieses[k].extensionName) == 0)
				{
					ext_supported = true;
				}
			}

			all_ext_supported |= ext_supported;
		}

		// Check features
		VkPhysicalDeviceFeatures fts_supported = {};
		vkGetPhysicalDeviceFeatures(examine_gpu, &fts_supported);

		// @formatter:off
		const bool all_ft_supported =
		fn_ext(fts_requested.robustBufferAccess, fts_supported.robustBufferAccess) &&
		fn_ext(fts_requested.fullDrawIndexUint32, fts_supported.fullDrawIndexUint32) &&
		fn_ext(fts_requested.imageCubeArray, fts_supported.imageCubeArray) &&
		fn_ext(fts_requested.independentBlend, fts_supported.independentBlend) &&
		fn_ext(fts_requested.geometryShader, fts_supported.geometryShader) &&
		fn_ext(fts_requested.tessellationShader, fts_supported.tessellationShader) &&
		fn_ext(fts_requested.sampleRateShading, fts_supported.sampleRateShading) &&
		fn_ext(fts_requested.dualSrcBlend, fts_supported.dualSrcBlend) &&
		fn_ext(fts_requested.logicOp, fts_supported.logicOp) &&
		fn_ext(fts_requested.multiDrawIndirect, fts_supported.multiDrawIndirect) &&
		fn_ext(fts_requested.drawIndirectFirstInstance, fts_supported.drawIndirectFirstInstance) &&
		fn_ext(fts_requested.depthClamp, fts_supported.depthClamp) &&
		fn_ext(fts_requested.depthBiasClamp, fts_supported.depthBiasClamp) &&
		fn_ext(fts_requested.fillModeNonSolid, fts_supported.fillModeNonSolid) &&
		fn_ext(fts_requested.depthBounds, fts_supported.depthBounds) &&
		fn_ext(fts_requested.wideLines, fts_supported.wideLines) &&
		fn_ext(fts_requested.largePoints, fts_supported.largePoints) &&
		fn_ext(fts_requested.alphaToOne, fts_supported.alphaToOne) &&
		fn_ext(fts_requested.multiViewport, fts_supported.multiViewport) &&
		fn_ext(fts_requested.samplerAnisotropy, fts_supported.samplerAnisotropy) &&
		fn_ext(fts_requested.textureCompressionETC2, fts_supported.textureCompressionETC2) &&
		fn_ext(fts_requested.textureCompressionASTC_LDR, fts_supported.textureCompressionASTC_LDR) &&
		fn_ext(fts_requested.textureCompressionBC, fts_supported.textureCompressionBC) &&
		fn_ext(fts_requested.occlusionQueryPrecise, fts_supported.occlusionQueryPrecise) &&
		fn_ext(fts_requested.pipelineStatisticsQuery, fts_supported.pipelineStatisticsQuery) &&
		fn_ext(fts_requested.vertexPipelineStoresAndAtomics, fts_supported.vertexPipelineStoresAndAtomics) &&
		fn_ext(fts_requested.fragmentStoresAndAtomics, fts_supported.fragmentStoresAndAtomics) &&
		fn_ext(fts_requested.shaderTessellationAndGeometryPointSize , fts_supported.shaderTessellationAndGeometryPointSize) &&
		fn_ext(fts_requested.shaderImageGatherExtended, fts_supported.shaderImageGatherExtended) &&
		fn_ext(fts_requested.shaderStorageImageExtendedFormats, fts_supported.shaderStorageImageExtendedFormats) &&
		fn_ext(fts_requested.shaderStorageImageMultisample, fts_supported.shaderStorageImageMultisample) &&
		fn_ext(fts_requested.shaderStorageImageReadWithoutFormat, fts_supported.shaderStorageImageReadWithoutFormat) &&
		fn_ext(fts_requested.shaderStorageImageWriteWithoutFormat, fts_supported.shaderStorageImageWriteWithoutFormat) &&
		fn_ext(fts_requested.shaderUniformBufferArrayDynamicIndexing, fts_supported.shaderUniformBufferArrayDynamicIndexing) &&
		fn_ext(fts_requested.shaderSampledImageArrayDynamicIndexing, fts_supported.shaderSampledImageArrayDynamicIndexing) &&
		fn_ext(fts_requested.shaderStorageBufferArrayDynamicIndexing, fts_supported.shaderStorageBufferArrayDynamicIndexing) &&
		fn_ext(fts_requested.shaderStorageImageArrayDynamicIndexing, fts_supported.shaderStorageImageArrayDynamicIndexing) &&
		fn_ext(fts_requested.shaderClipDistance, fts_supported.shaderClipDistance) &&
		fn_ext(fts_requested.shaderCullDistance, fts_supported.shaderCullDistance) &&
		fn_ext(fts_requested.shaderFloat64, fts_supported.shaderFloat64) &&
		fn_ext(fts_requested.shaderInt64, fts_supported.shaderInt64) &&
		fn_ext(fts_requested.shaderInt16, fts_supported.shaderInt16) &&
		fn_ext(fts_requested.shaderResourceResidency, fts_supported.shaderResourceResidency) &&
		fn_ext(fts_requested.shaderResourceMinLod, fts_supported.shaderResourceMinLod) &&
		fn_ext(fts_requested.sparseBinding, fts_supported.sparseBinding) &&
		fn_ext(fts_requested.sparseResidencyBuffer, fts_supported.sparseResidencyBuffer) &&
		fn_ext(fts_requested.sparseResidencyImage2D, fts_supported.sparseResidencyImage2D) &&
		fn_ext(fts_requested.sparseResidencyImage3D, fts_supported.sparseResidencyImage3D) &&
		fn_ext(fts_requested.sparseResidency2Samples, fts_supported.sparseResidency2Samples) &&
		fn_ext(fts_requested.sparseResidency4Samples, fts_supported.sparseResidency4Samples) &&
		fn_ext(fts_requested.sparseResidency8Samples, fts_supported.sparseResidency8Samples) &&
		fn_ext(fts_requested.sparseResidency16Samples, fts_supported.sparseResidency16Samples) &&
		fn_ext(fts_requested.sparseResidencyAliased, fts_supported.sparseResidencyAliased) &&
		fn_ext(fts_requested.variableMultisampleRate, fts_supported.variableMultisampleRate) &&
		fn_ext(fts_requested.inheritedQueries, fts_supported.inheritedQueries);
		// @formatter:on

		// Extensions and features must be supported from the gpu.
		gpus_rating[i] = ((all_ft_supported == VK_SUCCESS) && all_ext_supported)
			                 ? 40
			                 : -40;
		gpus_rating[i] += (gpu_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			                  ? 10
			                  : 0;
	}

	uint32_t selected_gpu_index = 0;

	for (uint32_t i = 1; i < gpu_count; i++)
	{
		if (gpus_rating[i] > gpus_rating[selected_gpu_index])
		{
			selected_gpu_index = i;
		}
	}

	// We are giving priority to features,
	// meaning that if the rating is negative, the selected gpu doesn't support the application requirements.
	// Therefor, the program should never continue if that's the case.
	VK_CHECK((gpus_rating[selected_gpu_index] < 0)
		? VK_ERROR_FEATURE_NOT_PRESENT
		: VK_SUCCESS);

	VkPhysicalDeviceProperties selected_gpu_prop = {};
	vkGetPhysicalDeviceProperties(gpus[selected_gpu_index], &selected_gpu_prop);

	std::printf("[VK] [GPU INFO]\n\t[NAME] %s\n\t[DEVICE ID] %u\n\t[VENDOR ID] %u",
	            selected_gpu_prop.deviceName,
	            selected_gpu_prop.deviceID,
	            selected_gpu_prop.vendorID);

	*p_gpu = gpus[selected_gpu_index];
}

uint32_t Graphics::FindMemoryTypeIdx(
	uint32_t                                type_filter,
	VkMemoryPropertyFlags                   memory_property_flags,
	const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties)
{
	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		const bool is_type_supported       = (type_filter & (1 << i)) != 0;
		const bool has_required_properties = (physical_device_memory_properties.memoryTypes[i].propertyFlags &
		                                      memory_property_flags)
		                                     == memory_property_flags;

		if (is_type_supported && has_required_properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

void Graphics::CreateShaderModule(
	VkDevice                 device,
	const std::vector<char>& shader_code,
	VkAllocationCallbacks*   p_allocator,
	VkShaderModule*          p_shader_module)
{
	const VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = static_cast<uint32_t>(shader_code.size()),
		.pCode = reinterpret_cast<const uint32_t*>(&shader_code[0]),
	};

	VK_CHECK(vkCreateShaderModule(
		device,
		&create_info,
		p_allocator,
		p_shader_module));
}

void Graphics::CreateBuffer(
	VkDevice                 device,
	VkPhysicalDevice         physical_device,
	VkDeviceSize             size,
	VkBufferUsageFlags       usage_flags,
	VkFormat                 format,
	VkMemoryPropertyFlagBits memory_property_flag_bits,
	VkAllocationCallbacks*   p_allocator,
	VkBuffer*                p_buffer,
	VkDeviceMemory*          p_memory)
{
	const VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = size,
		.usage = usage_flags,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
	};

	VK_CHECK(vkCreateBuffer(
		device,
		&buffer_create_info,
		p_allocator,
		p_buffer));

	VkMemoryRequirements mem_requirements = {};
	vkGetBufferMemoryRequirements(
		device,
		*p_buffer,
		&mem_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(
		physical_device,
		&memory_properties);

	const uint32_t memory_type_index = FindMemoryTypeIdx(
		mem_requirements.memoryTypeBits,
		memory_property_flag_bits,
		memory_properties);

	const VkMemoryAllocateInfo buffer_memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = memory_type_index
	};

	VK_CHECK(vkAllocateMemory(device, &buffer_memory_allocate_info, p_allocator, p_memory));
	VK_CHECK(vkBindBufferMemory(device, *p_buffer, *p_memory, 0));
}

void Graphics::CreateBufferView(
	VkDevice               device,
	VkBuffer               buffer,
	VkFormat               format,
	VkDeviceSize           offset,
	VkDeviceSize           range,
	VkAllocationCallbacks* p_allocator,
	VkBufferView*          p_buffer_view)
{
	const VkBufferViewCreateInfo buffer_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.buffer = buffer,
		.format = format,
		.offset = offset,
		.range = range,
	};

	VK_CHECK(vkCreateBufferView(
		device,
		&buffer_view_create_info,
		p_allocator,
		p_buffer_view));
}

void Graphics::CreateImage(
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
	VkDeviceMemory*          p_memory)
{
	const VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = image_type,
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = samples,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VK_CHECK(vkCreateImage(
		device,
		&image_info,
		nullptr,
		p_image));

	VkMemoryRequirements mem_requirements = {};
	vkGetImageMemoryRequirements(device, *p_image, &mem_requirements);

	VkPhysicalDeviceMemoryProperties memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	const uint32_t mem_type_idx = FindMemoryTypeIdx(
		mem_requirements.memoryTypeBits,
		memory_property_flag_bits,
		memory_properties);

	const VkMemoryAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = mem_type_idx,
	};

	VK_CHECK(vkAllocateMemory(
		device,
		&allocate_info,
		p_allocator,
		p_memory));

	VK_CHECK(vkBindImageMemory(
		device,
		*p_image,
		*p_memory,
		0));
}

void Graphics::CreateImageView(
	VkDevice               device,
	VkImage                image,
	VkImageViewType        view_type,
	VkFormat               format,
	VkComponentMapping     components,
	VkAllocationCallbacks* p_allocator,
	VkImageView*           p_image_view)
{
	const VkImageSubresourceRange image_subresource_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	const VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = image,
		.viewType = view_type,
		.format = format,
		.components = components,
		.subresourceRange = image_subresource_range,
	};

	VK_CHECK(vkCreateImageView(
		device,
		&image_view_create_info,
		p_allocator,
		p_image_view));
}