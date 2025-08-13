/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/os_detector.h>

struct zmk_preferred_os_state_changed {
  enum zmk_os_type os;
};

ZMK_EVENT_DECLARE(zmk_preferred_os_state_changed);