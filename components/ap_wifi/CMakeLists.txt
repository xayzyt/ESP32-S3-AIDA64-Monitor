<<<<<<< HEAD
idf_component_register(
    SRCS "ap_wifi.c" "ws_server.c" "wifi_manager.c"
    INCLUDE_DIRS    "."
    REQUIRES esp_wifi esp_http_server lwip json spiffs
=======
idf_component_register(
    SRCS "ap_wifi.c" "ws_server.c" "wifi_manager.c"
    INCLUDE_DIRS    "."
    REQUIRES esp_wifi esp_http_server lwip json spiffs
>>>>>>> 81fc6d11b1a3aa3674e1ca4f6c48bf0d832d09d3
)