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

## Swapchain Recreation

- Query swapchain extent
- Query swapchain image count
- Fill create info structure with old swapchain
- Create the new swapchain
- Destroy image views
- Destroy images (?)
- Destroy per-frame sync obejcts
- Destroy old swapchain
- Get swapchain images
- Create per frame sync objects
- Create image views

## Multisample

- Create an image with the desired multisample count.
- Create the image view associated to the image.
- Create **VkAttachmentDescription** with the desired multisample count.

This description must point to the image created with the desired sample count.

- Create **VkAttachmentDescription** with **VK_SAMPLE_COUNT_1_BIT**.

This description must point to the image queried from the swapchain.

- Create the **VkPipelineMultisampleStateCreateInfo** with the desired multisample.

## Screenshot

-

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
