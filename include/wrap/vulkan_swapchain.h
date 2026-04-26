#pragma once

#include "vulkan_types.h"


SwapChainSupportDetails vk_query_swap_chain_support(VkPhysicalDevice device);

void vk_free_swap_chain_support(SwapChainSupportDetails* details);

VkSurfaceFormatKHR vk_choose_swap_surface_format(VkSurfaceFormatKHR* formats, uint32_t count);

VkPresentModeKHR vk_choose_swap_present_mode(VkPresentModeKHR* modes, uint32_t count);

VkExtent2D vk_choose_swap_extent(const VkSurfaceCapabilitiesKHR* caps);

void vk_create_swap_chain(void);

void vk_destroy_swap_chain(void);

void vk_create_image_views(void);
