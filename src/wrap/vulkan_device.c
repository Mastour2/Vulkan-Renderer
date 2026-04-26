
#include "wrap/vulkan_device.h"
#include "wrap/vulkan_swapchain.h"

#include <stdlib.h>
#include <string.h>

//  Queue families
QueueFamilyIndices vk_find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices = { .graphics_family = -1, .present_family = -1 };

    if (vk.surface == VK_NULL_HANDLE) {
        SDL_Log("[Vulkan] vk_find_queue_families: surface not created yet!");
        return indices;
    }

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);

    VkQueueFamilyProperties* families = malloc(sizeof(VkQueueFamilyProperties) * count);
    if (!families) return indices;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families);

    for (uint32_t i = 0; i < count; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = (int)i;
        }

        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk.surface, &present);

        if (present) indices.present_family = (int)i;

        if (QUEUE_FAMILIES_COMPLETE(indices)) break;
    }

    free(families);
    return indices;
}

//  Device extension support
bool vk_check_device_extension_support(VkPhysicalDevice device) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &count, NULL);

    VkExtensionProperties* exts = malloc(sizeof(VkExtensionProperties) * count);
    if (!exts) return false;
    vkEnumerateDeviceExtensionProperties(device, NULL, &count, exts);

    bool found = false;
    for (uint32_t i = 0; i < count; i++) {
        if (strcmp(exts[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            found = true;
            break;
        }
    }

    free(exts);
    return found;
}

//  Device suitability
int vk_rate_device_suitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);

    int score = 0;
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score +=  500;
    return score;
}

bool vk_is_device_suitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = vk_find_queue_families(device);
    if (!QUEUE_FAMILIES_COMPLETE(indices)) return false;

    if (!vk_check_device_extension_support(device)) return false;

    SwapChainSupportDetails sc = vk_query_swap_chain_support(device);
    bool adequate = (sc.format_count > 0) && (sc.present_mode_count > 0);
    vk_free_swap_chain_support(&sc);

    return adequate;

}

//  Pick best physical device
void vk_pick_physical_device(void) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(vk.instance, &count, NULL);

    if (count == 0) {
        SDL_Log("[Vulkan] No GPUs with Vulkan support found!");
        return;
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * count);
    if (!devices) return;
    vkEnumeratePhysicalDevices(vk.instance, &count, devices);

    int   best_score  = -1;
    VkPhysicalDevice best_device = VK_NULL_HANDLE;

    for (uint32_t i = 0; i < count; i++) {
        if (!vk_is_device_suitable(devices[i])) continue;

        int score = vk_rate_device_suitability(devices[i]);
        if (score > best_score) {
            best_score  = score;
            best_device = devices[i];
        }
    }

    free(devices);

    if (best_device == VK_NULL_HANDLE) {
        SDL_Log("[Vulkan] No suitable GPU found!");
        return;
    }

    vk.physical_device = best_device;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vk.physical_device, &props);
    SDL_Log("[Vulkan] Selected GPU: %s (score: %d)", props.deviceName, best_score);
}

//  Create logical device + queues
void vk_create_logical_device(void) {
    QueueFamilyIndices indices = vk_find_queue_families(vk.physical_device);

    uint32_t unique_families[2] = {
        (uint32_t)indices.graphics_family,
        (uint32_t)indices.present_family
    };

    int num_unique = (indices.graphics_family == indices.present_family) ? 1 : 2;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_cis[2] = {0};
    for (int i = 0; i < num_unique; i++) {
        queue_cis[i].sType  = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[i].queueFamilyIndex = unique_families[i];
        queue_cis[i].queueCount = 1;
        queue_cis[i].pQueuePriorities = &priority;
    }

    VkPhysicalDeviceFeatures features = {0};

    const char* device_exts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount  = (uint32_t)num_unique,
        .pQueueCreateInfos = queue_cis,
        .pEnabledFeatures = &features,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = device_exts,
    };

    if (vkCreateDevice(vk.physical_device, &ci, NULL, &vk.device) != VK_SUCCESS) {
        SDL_Log("[Vulkan] Failed to create logical device!");
        return;
    }

    vkGetDeviceQueue(vk.device, (uint32_t)indices.graphics_family, 0, &vk.graphics_queue);
    vkGetDeviceQueue(vk.device, (uint32_t)indices.present_family,  0, &vk.present_queue);

    SDL_Log("[Vulkan] Logical device created.");
}
