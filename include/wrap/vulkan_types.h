#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdbool.h>
#include <stdint.h>

//  Validation layers
#define MAX_EXTENSIONS 64

extern const char* validation_layers[];
extern const uint32_t validation_layers_count;

#ifdef NDEBUG
    #define ENABLE_VALIDATION false
#else
    #define ENABLE_VALIDATION true
#endif

//  Queue families
typedef struct {
int graphics_family;
int present_family;
} QueueFamilyIndices;

#define QUEUE_FAMILIES_COMPLETE(q) ((q).graphics_family >= 0 && (q).present_family >= 0)

//  Swap chain support
typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t format_count;
    VkPresentModeKHR* present_modes;
    uint32_t present_mode_count;
} SwapChainSupportDetails;

//  Global Vulkan context
typedef struct {
    SDL_Window* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkSurfaceKHR surface;

    VkPhysicalDevice physical_device;
    VkDevice device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSwapchainKHR swap_chain;
    VkImage* swap_chain_images;
    uint32_t swap_chain_image_count;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;

    VkImageView* swap_chain_image_views;
    uint32_t swap_chain_image_view_count;
} VulkanContext;

// Global context (defined in main.c)
extern VulkanContext vk;
