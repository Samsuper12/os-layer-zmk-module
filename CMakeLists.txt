zephyr_include_directories(include)
zephyr_library()

zephyr_library_sources(src/usb_os_detector.c)
target_sources(app PRIVATE src/events/preferred_os_state_changed.c)
target_sources(app PRIVATE src/behaviors/behavior_os_selector.c)
target_sources(app PRIVATE src/behaviors/behavior_os_mod.c)