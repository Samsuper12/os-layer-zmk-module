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

#define OS_CHOOSE_MODE DEVICE_DT_NAME(DT_INST(0, zmk_os_choose))
#define ZM_IS_NODE_MATCH(a, b) (strcmp(a, b) == 0)
#define IS_CHOOSE_MODE(dev) ZM_IS_NODE_MATCH(dev, OS_CHOOSE_MODE)

struct behavior_os_mod_config {
  uint32_t count;
  struct zmk_behavior_binding bindings[];
};

struct behavior_os_mod_data {
  struct zmk_behavior_binding *pressed_binding;
};

static void query_bindings(const struct zmk_behavior_binding bindings[],
                           int bindings_count,
                           struct zmk_behavior_binding_event event,
                           struct behavior_os_mod_data *data) {
  const enum zmk_os_type preferred_os = zmk_get_preferred_os_type();
  enum zmk_os_type selected_os = 0;
  bool os_mode_found = false;

  for (size_t i = 0; i < bindings_count; ++i) {
    struct zmk_behavior_binding *binding = &bindings[i];
    if (IS_CHOOSE_MODE(binding->behavior_dev)) {
      selected_os = binding->param1;
      os_mode_found = true;
      continue;
    }

    if (os_mode_found && selected_os == preferred_os) {
      data->pressed_binding = binding;
      zmk_behavior_invoke_binding(binding, event, true);
    }
  }
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
  const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
  const struct behavior_os_mod_config *cfg = dev->config;
  struct behavior_os_mod_data *data = dev->data;
  query_bindings(cfg->bindings, cfg->count, event, data);
  return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
  const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
  struct behavior_os_mod_data *data = dev->data;
  zmk_behavior_invoke_binding(data->pressed_binding, event, false);
  data->pressed_binding = NULL;

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
  {LISTIFY(DT_INST_PROP_LEN(n, bindings), ZMK_KEYMAP_EXTRACT_BINDING2, (, ), n)}

#define OS_INST(n)                                                             \
  static struct behavior_os_mod_config behavior_os_mod_config_##n = {          \
      .count = DT_INST_PROP_LEN(n, bindings),                                  \
      .bindings = TRANSFORMED_BEHAVIORS(n)};                                   \
  static struct behavior_os_mod_data behavior_os_mod_data_##n = {};            \
  BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &behavior_os_mod_data_##n,            \
                          &behavior_os_mod_config_##n, POST_KERNEL,            \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                 \
                          &behavior_os_mod_driver_api);

DT_INST_FOREACH_STATUS_OKAY(OS_INST)
