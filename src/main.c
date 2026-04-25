#include "vulkan/vulkan_core.h"
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cglm/cglm.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define W 512
#define H 512

static SDL_Window *window;
VkInstance instance;
VkPhysicalDevice physical_device;
VkDevice device = VK_NULL_HANDLE;
VkQueue queue = VK_NULL_HANDLE;

void vk_create_instance() {
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t ext_count = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&ext_count);

    if(!extensions) {
        SDL_Log("Could not get Vulkan extensions: %s", SDL_GetError());
        return;
    }


    create_info.enabledExtensionCount = ext_count;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount = 0;
    create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;


    if (vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS) {
        SDL_Log("failed to create instance!");
        return;
    }

    SDL_Log("Vulkan initalized successfully!");
}

void vk_physical_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);

    if (device_count == 0) {
        SDL_Log("failed to find GPUs with Vulkan support!");
    }

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    physical_device = devices[0];

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_device, &deviceProperties);
    SDL_Log("GPU: %s", deviceProperties.deviceName);

    free(devices);
}

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    vk_create_instance();
    vk_physical_device();

    window = SDL_CreateWindow("Vulkan", W, H, 0);
}

void loop() {
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }
}

void cleanup() {
    vkDestroyInstance(instance, NULL);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    init();
    loop();
    cleanup();
    return 0;
}
