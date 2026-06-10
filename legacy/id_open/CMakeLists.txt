idf_component_register(
    SRCS 
        "id_open_esp32.cpp"
        "id_open.cpp"
        "id_open_beacon.cpp"
        "alt_unix_time.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        esp_wifi
        bt
        esp_event
        nvs_flash
        esp_system
        freertos
        driver
)

# Link C++ standard library
target_link_libraries(${COMPONENT_LIB} PUBLIC stdc++ m)
