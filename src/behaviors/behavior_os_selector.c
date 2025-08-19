/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_os_selector

#include <drivers/behavior.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <dt-bindings/zmk/os_layers_defines.h>
#include <zmk/keymap.h>
#include <zmk/os_layer.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_keymap_binding_convert_central_state_dependent_params(
    struct zmk_behavior_binding *binding,
    struct zmk_behavior_binding_event event) {

  switch (binding->param1) {
  case GO_NEXT_OS_CMD: {
    binding->param1 = OS_SET_CMD;
    binding->param2 = zmk_calc_next_os_type(+1);
    break;
  }
  case GO_PREV_OS_CMD: {
    binding->param1 = OS_SET_CMD;
    binding->param2 = zmk_calc_next_os_type(-1);
    break;
  }
  case KP_OS_LINUX: {
    binding->param1 = OS_SET_CMD;
    binding->param2 = ZMK_OS_LINUX;
    break;
  }
  case KP_OS_MAC: {
    binding->param1 = OS_SET_CMD;
    binding->param2 = ZMK_OS_MAC;
    break;
  }
  case KP_OS_WIN: {
    binding->param1 = OS_SET_CMD;
    binding->param2 = ZMK_OS_WIN;
    break;
  }
  default:
    return 0;
  }

  LOG_DBG("zmk_get_preferred_os_type: %i", zmk_get_preferred_os_type());

  return 0;
};

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {

  /*
    case GO_NEXT_OS_CMD:
return zmk_select_preferred_os_type(zmk_calc_next_os_type(+1));
case GO_PREV_OS_CMD:
return zmk_select_preferred_os_type(zmk_calc_next_os_type(-1));
*/

  LOG_DBG("binding->param1: %i, binding->param2: %i ", binding->param1,
          binding->param2);
  switch (binding->param1) {
  case KP_OS_LINUX:
    return zmk_select_preferred_os_type(ZMK_OS_LINUX);
  case KP_OS_MAC:
    return zmk_select_preferred_os_type(ZMK_OS_MAC);
  case KP_OS_WIN:
    return zmk_select_preferred_os_type(ZMK_OS_WIN);
  case OS_SET_CMD:
    return zmk_select_preferred_os_type(binding->param2);
  }

  return -ENOTSUP;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
  return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_os_detector_driver_api = {
    .binding_convert_central_state_dependent_params =
        on_keymap_binding_convert_central_state_dependent_params,
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_os_detector_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
