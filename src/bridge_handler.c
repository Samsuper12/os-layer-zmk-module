/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zmk/bridge.h>
#include <zmk/os_layer.h>

bridge_Response get_devicset_current_ose_info(const bridge_Request *req) {

  zmk_select_preferred_os_type(
      (enum zmk_os_type)req->subsystem.os.request_type.set_current_os);

  return BRIDGE_RESPONSE_OK;
}

BRIDGE_SUBSYSTEM_HANDLER(os, set_current_os);