#pragma once

#include "vulkan_types.h"

QueueFamilyIndices vk_find_queue_families(VkPhysicalDevice device);

bool vk_check_device_extension_support(VkPhysicalDevice device);

bool vk_is_device_suitable(VkPhysicalDevice device);

int  vk_rate_device_suitability(VkPhysicalDevice device);

void vk_pick_physical_device(void);

void vk_create_logical_device(void);
