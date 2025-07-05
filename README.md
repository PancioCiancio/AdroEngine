# ADRO GFX (notes)

## Vulkan Handles List Ordered

- VkInstance
- VkSurfaceKHR
- VkPhysicalDevice
- VkDevice
- VkSwapchainKHR
- VkImage
- VkImageView
- VkRenderPass
- VkPipelineLayout
- VkShaderModule
- VkPipeline
- VkFramebuffer
- VkCommandPool
- VkCommandBuffer
- Synchronization Primitives

## Platform Dependent Layer

Consider to separate the interface from its implementation if both of them
are platform dependent (Bridge Pattern).

### Windows

- vkGetPhysicalDeviceWin32PresentationSupportKHR
- vkCreateWin32SurfaceKHR
- VkWin32SurfaceCreateInfoKHR

### Xlib

- vkGetPhysicalDeviceXlibPresentationSupportKHR
- vkCreateXlibSurfaceKHR
- VkXlibSurfaceCreateInfoKHR
