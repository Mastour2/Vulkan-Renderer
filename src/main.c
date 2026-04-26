#include "wrap/vulkan_types.h"
#include "wrap/vulkan_instance.h"
#include "wrap/vulkan_device.h"
#include "wrap/vulkan_swapchain.h"

// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>

#define W 512
#define H 512

VulkanContext vk = {0};

static void init_window(void) {
    SDL_Init(SDL_INIT_VIDEO);
    vk.window = SDL_CreateWindow("Vulkan", W, H, SDL_WINDOW_VULKAN);
}

static void init(void) {
    init_window();

    vk_create_instance();
    vk_setup_debug_messenger();

    if (!SDL_Vulkan_CreateSurface(vk.window, vk.instance, NULL, &vk.surface)) {
        SDL_Log("[SDL] Failed to create Vulkan surface: %s", SDL_GetError());
        return;
    }

    vk_pick_physical_device();
    vk_create_logical_device();
    vk_create_swap_chain();
}


static void loop(void) {
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }
    }
}


static void cleanup(void) {
    vk_destroy_swap_chain();

    if (vk.device != VK_NULL_HANDLE) {
        vkDestroyDevice(vk.device, NULL);
    }

    if (vk.surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(vk.instance, vk.surface, NULL);
    }

    vk_destroy_debug_messenger();

    if (vk.instance != VK_NULL_HANDLE) {
        vkDestroyInstance(vk.instance, NULL);
    }

    SDL_DestroyWindow(vk.window);
    SDL_Quit();
}

int main(void) {
    init();
    loop();
    cleanup();
    return 0;
}
