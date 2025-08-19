/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

enum zmk_os_type {
  ZMK_OS_LINUX = 0,
  ZMK_OS_MAC = 1,
  ZMK_OS_WIN = 2,

  ZMK_OS_OTHER = 20,
};

int zmk_select_preferred_os_type(enum zmk_os_type);
int zmk_calc_next_os_type(int);
enum zmk_os_type zmk_get_preferred_os_type(void);
