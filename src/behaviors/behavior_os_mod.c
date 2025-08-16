/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_os_mod

#include <drivers/behavior.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior_queue.h>

#include <dt-bindings/zmk/os_detector_defines.h>
#include <zmk/keymap.h>
#include <zmk/os_detector.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_os_mod_config {
  uint32_t count;
  struct zmk_behavior_binding bindings[];
};

static void queue_os(const struct zmk_behavior_binding bindings[],
                     struct zmk_behavior_binding_event *event, bool press,
                     int count) {
  const enum zmk_os_type current_os = zmk_get_preferred_os_type();

  for (size_t i = 0; i < count; ++i) {
    struct zmk_behavior_binding binding = bindings[i];
    if (binding.param2 == current_os) {
      LOG_DBG("New OS-Mod event. param1: %i, param2: %i. Press? %i",
              binding.param1, binding.param2, press);
      zmk_behavior_queue_add(event, binding, press, 15);
    }
  }
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
  const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
  const struct behavior_os_mod_config *cfg = dev->config;
  queue_os(cfg->bindings, &event, true, cfg->count);
  return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
  const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
  const struct behavior_os_mod_config *cfg = dev->config;
  queue_os(cfg->bindings, &event, true, cfg->count);
  return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_os_mod_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define ZMK_KEYMAP_EXTRACT_BINDING2(idx, node)                                 \
  {                                                                            \
      .behavior_dev =                                                          \
          DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(node, bindings, idx)),         \
      .param1 = COND_CODE_0(                                                   \
          DT_INST_PHA_HAS_CELL_AT_IDX(node, bindings, idx, param1), (0),       \
          (DT_INST_PHA_BY_IDX(node, bindings, idx, param1))),                  \
      .param2 = COND_CODE_0(                                                   \
          DT_INST_PHA_HAS_CELL_AT_IDX(node, bindings, idx, param2), (0),       \
          (DT_INST_PHA_BY_IDX(node, bindings, idx, param2))),                  \
  }

#define TRANSFORMED_BEHAVIORS(n)                                               \
  {LISTIFY(DT_PROP_LEN(n, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), n)}

#define DYN_INST(inst)                                                         \
  static struct behavior_os_mod_config behavior_os_mod_config_##inst = {       \
      .count = DT_PROP_LEN(inst, bindings),                                    \
      .bindings = TRANSFORMED_BEHAVIORS(inst)};                                \
  BEHAVIOR_DT_INST_DEFINE(                                                     \
      inst, NULL, NULL, NULL, &behavior_os_mod_config_##inst, POST_KERNEL,     \
      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_os_mod_driver_api);

DT_FOREACH_STATUS_OKAY(zmk_behavior_os_mod_one_param, DYN_INST)
