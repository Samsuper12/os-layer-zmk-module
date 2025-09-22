/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zmk/bridge.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/os_layer.h>
#include <zmk/virtual_key_position.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);
BRIDGE_DECLARE_BINDING(binding, event, os_sel);

bridge_Response set_current_os(const bridge_Request *req) {

  if (!device_is_ready(os_sel_dev)) {
    LOG_ERR("The OS_SEL node cannot be found!");
    return BRIDGE_RESPONSE_SIMPLE(false);
  }

  binding.param1 = req->subsystem.os.request_type.set_current_os;
  bridge_tap_binding(&binding, event);

  return BRIDGE_RESPONSE_SIMPLE(true);
}

BRIDGE_SUBSYSTEM_HANDLER(os, set_current_os);