idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        arduino-esp32
        smartcooling
        esp_wifi
        esp_netif
        nvs_flash
        app_update
        mdns
)
