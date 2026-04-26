#include "wrap/vulkan_swapchain.h"
#include "wrap/vulkan_device.h"

#include <stdlib.h>

//  Query swap chain support details
SwapChainSupportDetails vk_query_swap_chain_support(VkPhysicalDevice device) {
SwapChainSupportDetails d = {0};

vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vk.surface, &d.capabilities);

vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk.surface, &d.format_count, NULL);
if (d.format_count > 0) {
    d.formats = malloc(sizeof(VkSurfaceFormatKHR) * d.format_count);
    if (d.formats)
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk.surface, &d.format_count, d.formats);
}

vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk.surface, &d.present_mode_count, NULL);
if (d.present_mode_count > 0) {
    d.present_modes = malloc(sizeof(VkPresentModeKHR) * d.present_mode_count);
    if (d.present_modes)
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vk.surface, &d.present_mode_count, d.present_modes);
}

return d;
}

void vk_free_swap_chain_support(SwapChainSupportDetails* d) {
    if (!d) return;

    free(d->formats);
    d->formats = NULL;
    free(d->present_modes);

    d->present_modes = NULL;
    d->format_count = 0;
    d->present_mode_count = 0;
}

//  Selection helpers
VkSurfaceFormatKHR vk_choose_swap_surface_format(VkSurfaceFormatKHR* formats, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
    formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    return formats[i];
    }
    return formats[0];
}

VkPresentModeKHR vk_choose_swap_present_mode(VkPresentModeKHR* modes, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        return modes[i];
    }
    return VK_PRESENT_MODE_FIFO_KHR;   // guaranteed to be available
}

VkExtent2D vk_choose_swap_extent(const VkSurfaceCapabilitiesKHR* caps) {
    // If the surface dictates a fixed extent, use it directly
    if (caps->currentExtent.width != 0xFFFFFFFF) {
        return caps->currentExtent;
    }

    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(vk.window, &w, &h);

    VkExtent2D extent = { (uint32_t)w, (uint32_t)h };
    extent.width  = SDL_clamp(extent.width,  caps->minImageExtent.width,  caps->maxImageExtent.width);
    extent.height = SDL_clamp(extent.height, caps->minImageExtent.height, caps->maxImageExtent.height);
    return extent;

}

//  Create swap chain
void vk_create_swap_chain(void) {
    SwapChainSupportDetails sc = vk_query_swap_chain_support(vk.physical_device);

    VkSurfaceFormatKHR fmt  = vk_choose_swap_surface_format(sc.formats, sc.format_count);
    VkPresentModeKHR mode = vk_choose_swap_present_mode(sc.present_modes, sc.present_mode_count);
    VkExtent2D ext  = vk_choose_swap_extent(&sc.capabilities);

    uint32_t img_count = sc.capabilities.minImageCount + 1;
    if (sc.capabilities.maxImageCount > 0 && img_count > sc.capabilities.maxImageCount) {
        img_count = sc.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR ci = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = vk.surface,
        .minImageCount    = img_count,
        .imageFormat      = fmt.format,
        .imageColorSpace  = fmt.colorSpace,
        .imageExtent      = ext,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform     = sc.capabilities.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = mode,
        .clipped          = VK_TRUE,
        .oldSwapchain     = VK_NULL_HANDLE,
    };

    QueueFamilyIndices indices = vk_find_queue_families(vk.physical_device);
    uint32_t families[] = { (uint32_t)indices.graphics_family, (uint32_t)indices.present_family };

    if (indices.graphics_family != indices.present_family) {
        ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        ci.queueFamilyIndexCount = 2;
        ci.pQueueFamilyIndices   = families;
    } else {
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (vkCreateSwapchainKHR(vk.device, &ci, NULL, &vk.swap_chain) != VK_SUCCESS) {
        SDL_Log("[Vulkan] Failed to create swap chain!");
        vk_free_swap_chain_support(&sc);
        return;
    }

    // Retrieve swap chain images
    vkGetSwapchainImagesKHR(vk.device, vk.swap_chain, &vk.swap_chain_image_count, NULL);
    vk.swap_chain_images = malloc(sizeof(VkImage) * vk.swap_chain_image_count);
    vkGetSwapchainImagesKHR(vk.device, vk.swap_chain, &vk.swap_chain_image_count, vk.swap_chain_images);

    vk.swap_chain_image_format = fmt.format;
    vk.swap_chain_extent = ext;

    SDL_Log("[Vulkan] Swap chain created (%ux%u, %u images).", ext.width, ext.height, vk.swap_chain_image_count);

    vk_free_swap_chain_support(&sc);
}

//  Destroy swap chain
void vk_destroy_swap_chain(void) {
    if (vk.swap_chain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vk.device, vk.swap_chain, NULL);
        vk.swap_chain = VK_NULL_HANDLE;
    }
    free(vk.swap_chain_images);
    vk.swap_chain_images = NULL;
}
