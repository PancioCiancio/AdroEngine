# ADRO GFX (notes)

## Plan

- [ ] Memory
- [x] Triangle
- [ ] Model
- [x] Camera
- [ ] Smooth Camera
- [ ] Batch Rendering
- [ ] Input System
- [ ] Mouse Picking
- [ ] Gui Integration

## Bans

- Custom allocator for Vulkan resources.

## Vulkan Initialization

### Offline Rendering

1. VkInstance
2. VkDebugMessenger
4. VkPhysicalDevice
5. VkDevice
7. VkImage
8. VkImageView
9. VkRenderPass
10. VkPipelineLayout
11. VkShaderModule
12. VkPipeline
13. VkFramebuffer
14. VkCommandPool
15. VkCommandBuffer
16. VkSemaphore
17. VkFence

### Realtime Rendering

1. ...
2. VkDebugMessenger
3. VkSurfaceKHR (display extension)
4. VkPhysicalDevice
5. VkDevice
6. VkSwapchainKHR (display extension)
7. ...

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
