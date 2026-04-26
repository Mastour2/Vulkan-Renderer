#pragma once

#include "vulkan_types.h"

bool vk_check_validation_layer_support(void);

const char** vk_get_required_extensions(uint32_t* out_count);

void vk_create_instance(void);

void vk_setup_debug_messenger(void);

void vk_destroy_debug_messenger(void);
