//
// Created by apant on 25/07/2025.
//

#include "vk_physical_device.h"
#include "vk_utils.h"

#include <cstring>
#include <iostream>

namespace Gfx
{

void QueryGpu(
	VkInstance               instance,
	VkPhysicalDeviceFeatures fts_requested,
	uint32_t                 requested_extension_count,
	const char**             p_requested_extensions,
	VkPhysicalDevice*        p_gpu)
{
	uint32_t gpu_count = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(
		instance,
		&gpu_count,
		nullptr));

	VkPhysicalDevice gpus[16] = {};
	VK_CHECK(vkEnumeratePhysicalDevices(
		instance,
		&gpu_count,
		&gpus[0]))

	;

	// Helper lambda to compare feature requested to supported.
	auto fn_ext = [](const VkBool32 requested, const VkBool32 supported) -> VkResult
	{
		return (requested == VK_TRUE && supported == VK_FALSE)
			       ? VK_ERROR_FEATURE_NOT_PRESENT
			       : VK_SUCCESS;
	};

	// Alghoritm
	int32_t gpus_rating[16] = {0};

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

		VkExtensionProperties gpu_extension_properties[256] = {};
		VK_CHECK(vkEnumerateDeviceExtensionProperties(
			examine_gpu,
			nullptr,
			&gpu_extension_count,
			&gpu_extension_properties[0]));

		// Check extensions.
		bool all_ext_supported = false;

		for (uint32_t j = 0; j < requested_extension_count; j++)
		{
			const char* requested_extension = p_requested_extensions[j];

			bool ext_supported = false;

			for (uint32_t k = 0; k < gpu_extension_count && !ext_supported; k++)
			{
				if (std::strcmp(requested_extension, gpu_extension_properties[k].extensionName) == 0)
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

void QuerySampleCounts(
	VkPhysicalDevice       gpu,
	VkSampleCountFlagBits* p_sample)
{
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(gpu, &properties);

	const uint32_t sample_counts =
		properties.limits.framebufferColorSampleCounts &
		properties.limits.framebufferDepthSampleCounts &
		properties.limits.framebufferStencilSampleCounts;

	constexpr uint32_t              sample_bits_list_count                       = 4;
	constexpr VkSampleCountFlagBits priority_sample_bits[sample_bits_list_count] = {
		VK_SAMPLE_COUNT_8_BIT,
		VK_SAMPLE_COUNT_4_BIT,
		VK_SAMPLE_COUNT_2_BIT,
		VK_SAMPLE_COUNT_1_BIT
	};

	bool sample_count_found = false;
	for (uint32_t i = 0; i < sample_bits_list_count && !sample_count_found; i++)
	{
		if (priority_sample_bits[i] & sample_counts)
		{
			*p_sample          = priority_sample_bits[i];
			sample_count_found = true;
		}
	}
}

void QueryQueueFamily(
	VkPhysicalDevice gpu,
	VkQueueFlagBits  queue_flag_bits_requested,
	const bool       must_support_presentation,
	const uint32_t   family_idx_discarded_count,
	const uint32_t*  family_idx_discarded,
	uint32_t*        p_queue_family_idx)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);

	VkQueueFamilyProperties queue_families[32] = {};
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, &queue_families[0]);

	bool is_queue_family_found = false;

	for (uint32_t i = 0; i < queue_family_count && !is_queue_family_found; i++)
	{
		bool is_queue_discarded = false;

		for (uint32_t j = 0; j < family_idx_discarded_count && !is_queue_discarded; j++)
		{
			is_queue_discarded = family_idx_discarded[j] == i;
		}

		*p_queue_family_idx = i;

		// Check presentation support if requested, otherwise mark it as true.
		const bool support_presentation = must_support_presentation
			                                  ? vkGetPhysicalDeviceWin32PresentationSupportKHR(gpu, i)
			                                  : true;

		// Check flags, presentation support, unique idx(if requested).
		is_queue_family_found = !is_queue_discarded &&
		                        support_presentation &&
		                        (queue_families[i].queueFlags & queue_flag_bits_requested) ==
		                        queue_flag_bits_requested;
	}

	VK_CHECK(is_queue_family_found
		? VK_SUCCESS
		: VK_ERROR_INITIALIZATION_FAILED);
}

void QuerySurfaceFormat(
	VkPhysicalDevice    gpu,
	VkSurfaceKHR        surface,
	const uint32_t      required_format_count,
	const VkFormat*     p_required_formats,
	VkSurfaceFormatKHR* p_format)
{
	uint32_t surface_format_count = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
		gpu,
		surface,
		&surface_format_count,
		nullptr));

	VK_CHECK((surface_format_count > 0) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

	VkSurfaceFormatKHR formats_supported[32] = {};
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
		gpu,
		surface,
		&surface_format_count,
		&formats_supported[0]));

	bool find_required_format = false;

	for (uint32_t i = 0; i < surface_format_count && !find_required_format; i++)
	{
		for (uint32_t j = 0; j < required_format_count && !find_required_format; j++)
		{
			find_required_format = formats_supported[i].format == p_required_formats[j];

			if (find_required_format)
			{
				*p_format = formats_supported[i];
			}
		}
	}

	// If no required format is provided or found, assign the first one supported.
	if (!find_required_format)
	{
		*p_format = formats_supported[0];
	}
}

void QuerySurfaceCapabilities(
	VkPhysicalDevice          gpu,
	VkSurfaceKHR              surface,
	VkSurfaceCapabilitiesKHR* p_surface_capabilities)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, p_surface_capabilities);

	constexpr uint32_t desired_image_count = 3;
	p_surface_capabilities->minImageCount  = desired_image_count;

	if (p_surface_capabilities->maxImageCount > 0 &&
	    desired_image_count > p_surface_capabilities->maxImageCount)
	{
		p_surface_capabilities->minImageCount = p_surface_capabilities->maxImageCount;
	}

	if (p_surface_capabilities->supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		p_surface_capabilities->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
}

}