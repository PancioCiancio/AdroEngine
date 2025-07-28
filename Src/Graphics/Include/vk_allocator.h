//
// Created by apant on 28/07/2025.
//

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <volk/volk.h>

namespace Gfx
{
/// Example class of an allocator compatible with Vulkan.
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
}

#endif //ALLOCATOR_H
