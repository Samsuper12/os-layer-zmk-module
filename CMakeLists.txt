zephyr_include_directories(include)
zephyr_library()

zephyr_library_sources(src/usb_os_detector.c)
zephyr_library_sources(src/events/preferred_os_state_changed.c)
zephyr_library_sources(src/behaviors/behavior_os_selector.c)
zephyr_library_sources(src/behaviors/behavior_os_mod.c)


if (CONFIG_ZMK_BRIDGE)
    zephyr_library_sources(src/bridge_handler.c)
    zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR}/proto)
    zephyr_library_link_libraries(bridge-zmk-module)
endif()
