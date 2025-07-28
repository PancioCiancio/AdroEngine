//
// Created by apant on 28/07/2025.
//

#include "vk_allocator.h"

#include <memory>

namespace Gfx
{

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
}