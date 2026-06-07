idf_component_register(
    SRCS "firework.c" "main.c" "fill.c" "flipdot.c" "scroll.c" "snake.c" "text.c"
         "wifi_manager.c" "web_server.c"
    INCLUDE_DIRS "include"
    EMBED_TXTFILES "web/index.html"
    REQUIRES esp_driver_gpio esp_wifi esp_http_server nvs_flash esp_netif esp_event
)
