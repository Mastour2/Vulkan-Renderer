#include "wrap/vulkan_instance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  Validation layer names
const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const uint32_t validation_layers_count =
sizeof(validation_layers) / sizeof(validation_layers[0]);

//  Internal: extension buffer
static const char* s_extensions[MAX_EXTENSIONS];
static uint32_t    s_extension_count = 0;

//  Validation layer support check
bool vk_check_validation_layer_support(void) {
uint32_t count = 0;
vkEnumerateInstanceLayerProperties(&count, NULL);

VkLayerProperties* available = malloc(sizeof(VkLayerProperties) * count);
if (!available) return false;
vkEnumerateInstanceLayerProperties(&count, available);

for (uint32_t i = 0; i < validation_layers_count; i++) {
    bool found = false;
    for (uint32_t j = 0; j < count; j++) {
        if (strcmp(validation_layers[i], available[j].layerName) == 0) {
            found = true;
            break;
        }
    }
    if (!found) {
        free(available);
        return false;
    }
}

free(available);
return true;
}

//  Required instance extensions
const char** vk_get_required_extensions(uint32_t* out_count) {
uint32_t sdl_count = 0;
const char* const* sdl_exts = SDL_Vulkan_GetInstanceExtensions(&sdl_count);

s_extension_count = 0;

for (uint32_t i = 0; i < sdl_count && s_extension_count < MAX_EXTENSIONS; i++)
    s_extensions[s_extension_count++] = sdl_exts[i];

if (ENABLE_VALIDATION && s_extension_count < MAX_EXTENSIONS)
    s_extensions[s_extension_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

#ifdef __APPLE__
    if (s_extension_count < MAX_EXTENSIONS) {
        s_extensions[s_extension_count++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    }
#endif

*out_count = s_extension_count;
return s_extensions;
}

//  Debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL s_debug_callback(
VkDebugUtilsMessageSeverityFlagBitsEXT  severity,
VkDebugUtilsMessageTypeFlagsEXT         type,
const VkDebugUtilsMessengerCallbackDataEXT* data,
void* user_data
) {
    (void)type; (void)user_data;

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        SDL_Log("[Vulkan] %s", data->pMessage);
    }

    return VK_FALSE;
}

static void s_fill_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT* info) {
memset(info, 0, sizeof(*info));
info->sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
info->messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
info->pfnUserCallback = s_debug_callback;
}

//  Create VkInstance
void vk_create_instance(void) {
if (ENABLE_VALIDATION && !vk_check_validation_layer_support()) {
SDL_Log("[Vulkan] Validation layers requested but not available!");
return;
}

VkApplicationInfo app_info = {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = "Vulkan App",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName        = "No Engine",
    .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion         = VK_API_VERSION_1_0,
};

uint32_t    ext_count = 0;
const char** exts     = vk_get_required_extensions(&ext_count);

SDL_Log("[Vulkan] Enabled instance extensions (%u):", ext_count);
for (uint32_t i = 0; i < ext_count; i++)
    SDL_Log("  %s", exts[i]);

VkDebugUtilsMessengerCreateInfoEXT debug_info = {0};

VkInstanceCreateInfo ci = {
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo        = &app_info,
    .enabledExtensionCount   = ext_count,
    .ppEnabledExtensionNames = exts,
    #ifdef __APPLE__
        .flags  = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
    #endif
};

if (ENABLE_VALIDATION) {
    ci.enabledLayerCount   = validation_layers_count;
    ci.ppEnabledLayerNames = validation_layers;
    s_fill_debug_create_info(&debug_info);
    ci.pNext = &debug_info;
}

if (vkCreateInstance(&ci, NULL, &vk.instance) != VK_SUCCESS)
    SDL_Log("[Vulkan] Failed to create instance!");
else
    SDL_Log("[Vulkan] Instance created successfully.");
}

//  Debug messenger lifecycle
void vk_setup_debug_messenger(void) {
if (!ENABLE_VALIDATION) return;

VkDebugUtilsMessengerCreateInfoEXT info = {0};
s_fill_debug_create_info(&info);

PFN_vkCreateDebugUtilsMessengerEXT fn =
    (PFN_vkCreateDebugUtilsMessengerEXT)
    vkGetInstanceProcAddr(vk.instance, "vkCreateDebugUtilsMessengerEXT");

if (!fn || fn(vk.instance, &info, NULL, &vk.debug_messenger) != VK_SUCCESS)
    SDL_Log("[Vulkan] Failed to set up debug messenger!");
else
    SDL_Log("[Vulkan] Debug messenger ready.");

}

void vk_destroy_debug_messenger(void) {
if (!ENABLE_VALIDATION) return;

PFN_vkDestroyDebugUtilsMessengerEXT fn =
    (PFN_vkDestroyDebugUtilsMessengerEXT)
    vkGetInstanceProcAddr(vk.instance, "vkDestroyDebugUtilsMessengerEXT");

if (fn) fn(vk.instance, vk.debug_messenger, NULL);

}
