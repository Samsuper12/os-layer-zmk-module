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
#include <zmk/os_layer.h>
#include <zmk/virtual_key_position.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define OS_SEL_NODE DT_NODELABEL(os_sel)
const struct device *os_sel_dev = DEVICE_DT_GET(OS_SEL_NODE);

static struct zmk_behavior_binding behavior = {
    .behavior_dev = NULL,
    .param1 = 0,
    .param2 = 0,
};

bridge_Response set_current_os(const bridge_Request *req) {

  if (!device_is_ready(os_sel_dev)) {
    LOG_ERR("Can't find OS_SEL node!");
  }

  struct zmk_behavior_binding_event event = {
      .position = ZMK_VIRTUAL_KEY_POSITION_COMBO(1),
      .timestamp = k_uptime_get(),
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
      .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
  };

  behavior.behavior_dev = os_sel_dev;
  behavior.param1 = req->subsystem.os.request_type.set_current_os;

  zmk_behavior_invoke_binding(&behavior, event, true);
  k_msleep(2);
  zmk_behavior_invoke_binding(&behavior, event, false);

  return BRIDGE_RESPONSE_SIMPLE(true);
}

BRIDGE_SUBSYSTEM_HANDLER(os, set_current_os);