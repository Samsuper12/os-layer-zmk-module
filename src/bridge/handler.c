/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zmk/behavior.h>
#include <zmk/bridge.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/os_layer.h>
#include <zmk/virtual_key_position.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define OS_SEL_NODE DT_NODELABEL(os_sel)
static const struct device *os_sel_dev = DEVICE_DT_GET(OS_SEL_NODE);

static struct zmk_behavior_binding behavior = {
    .behavior_dev = NULL,
    .param1 = 0,
    .param2 = 0,
};

bridge_Response set_current_os(const bridge_Request *req) {

  if (!device_is_ready(os_sel_dev)) {
    LOG_ERR("The OS_SEL node cannot be found!");
  }

  struct zmk_behavior_binding_event event = {
      .position = 0,
      .timestamp = k_uptime_get(),
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
      .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
  };
  uint16_t os_layer = req->subsystem.os.request_type.set_current_os;
  behavior.behavior_dev = os_sel_dev;
  behavior.param1 = os_layer;
  zmk_select_preferred_os_type(os_layer);

  // In this case, the event (keypress) should be sent to all parts of the
  // keyboard.
  // Common events such as 'raise_' cannot do this.
  zmk_split_central_invoke_behavior(ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
                                    &behavior, event, true);
  k_msleep(2);
  event.timestamp = k_uptime_get();
  zmk_split_central_invoke_behavior(ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
                                    &behavior, event, false);

  return BRIDGE_RESPONSE_SIMPLE(true);
}

BRIDGE_SUBSYSTEM_HANDLER(os, set_current_os);