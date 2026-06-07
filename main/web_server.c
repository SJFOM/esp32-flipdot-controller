#include "web_server.h"
#include "main.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "web_server";

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

static httpd_handle_t server = NULL;

static esp_err_t root_get_handler(httpd_req_t *req)
{
    size_t len = index_html_end - index_html_start - 1; // -1: EMBED_TXTFILES adds null terminator
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, len);
    return ESP_OK;
}

static esp_err_t modes_get_handler(httpd_req_t *req)
{
    // Build JSON array: [{"id":0,"name":"SCROLL"}, ...]
    char buf[256] = "[";
    for (int i = 0; i < MODE_MAX; i++) {
        char item[64];
        snprintf(item, sizeof(item), "%s{\"id\":%d,\"name\":\"%s\"}",
                 i > 0 ? "," : "", i, mode_name((sys_mode_t)i));
        strlcat(buf, item, sizeof(buf));
    }
    strlcat(buf, "]", sizeof(buf));

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, buf);
    return ESP_OK;
}

static esp_err_t mode_get_handler(httpd_req_t *req)
{
    sys_mode_t m = mode_get();
    char buf[80];
    snprintf(buf, sizeof(buf), "{\"mode\":%d,\"name\":\"%s\"}", (int)m, mode_name(m));
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, buf);
    return ESP_OK;
}

static esp_err_t mode_post_handler(httpd_req_t *req)
{
    char body[64] = {0};
    int received = httpd_req_recv(req, body, sizeof(body) - 1);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty body");
        return ESP_FAIL;
    }

    // Parse {"mode": N} — locate the value after the "mode" key
    int new_mode = -1;
    const char *key = strstr(body, "\"mode\"");
    if (key) {
        const char *colon = strchr(key, ':');
        if (colon) new_mode = atoi(colon + 1);
    }
    if (new_mode < 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'mode' field");
        return ESP_FAIL;
    }

    if (new_mode < 0 || new_mode >= MODE_MAX) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Mode out of range");
        return ESP_FAIL;
    }

    mode_set((sys_mode_t)new_mode);
    ESP_LOGI(TAG, "Mode set to %d (%s) via web", new_mode, mode_name((sys_mode_t)new_mode));

    char buf[80];
    snprintf(buf, sizeof(buf), "{\"mode\":%d,\"name\":\"%s\"}", new_mode, mode_name((sys_mode_t)new_mode));
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, buf);
    return ESP_OK;
}

static const httpd_uri_t uri_root = {
    .uri = "/", .method = HTTP_GET, .handler = root_get_handler
};
static const httpd_uri_t uri_modes_get = {
    .uri = "/modes", .method = HTTP_GET, .handler = modes_get_handler
};
static const httpd_uri_t uri_mode_get = {
    .uri = "/mode", .method = HTTP_GET, .handler = mode_get_handler
};
static const httpd_uri_t uri_mode_post = {
    .uri = "/mode", .method = HTTP_POST, .handler = mode_post_handler
};

esp_err_t web_server_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    httpd_register_uri_handler(server, &uri_root);
    httpd_register_uri_handler(server, &uri_modes_get);
    httpd_register_uri_handler(server, &uri_mode_get);
    httpd_register_uri_handler(server, &uri_mode_post);

    return ESP_OK;
}

void web_server_stop(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}
