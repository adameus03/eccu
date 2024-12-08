#include <WiFi.h>
#include "esp_http_server.h"
//%%#include "esp_https_server.h" //$$

#include "ota.hpp" // for public OTA toggler
//#include "synchro.hpp"

//#include "time.h"

//#include <mbedtls>
//#include <mbedtls/esp_debug.h>
//#include "mbedtls/debug.h" //debug TLS handshake


httpd_handle_t server_httpd = NULL;

/* Non-local OTA */
static esp_err_t ota_upload_handler(httpd_req_t* req){
    //httpd
    /*const char *response = "Hello OTA. Not implemented";
    httpd_resp_send(req, response, strlen(response));*/


    char query[128];
    char state_param[4];

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        if (httpd_query_key_value(query, "state", state_param, sizeof(state_param)) != ESP_OK) {
            const char *response = "No state param or error reading state param.";
            httpd_resp_send(req, response, strlen(response));
            return ESP_OK;
        }
    }
    else {
        //const char *response = "Invalid query.";
        const char *response =
#include "htdocs/publicOTAToggle.html"
            ;
        httpd_resp_send(req, response, strlen(response));
        return ESP_OK;
    }

    if(!strcmp("on", state_param)){
        resumePublicOTA();
        const char *response = "PublicOTA is on";
        httpd_resp_send(req, response, strlen(response));
    }
    else if(!strcmp("off", state_param)){
        haltPublicOTA();
        const char *response = "PublicOTA is off";
        httpd_resp_send(req, response, strlen(response));
    }   
    else {
        const char *response = "Wrong state value provided.";
        httpd_resp_send(req, response, strlen(response));
    }
    return ESP_OK; //{{{return res instead?}}}
}



void startWebServer(){  //https://github.com/espressif/esp-idf/blob/82cceabc6e6a0a2d8c40e2249bc59917cc5e577a/examples/protocols/https_server/simple/main/main.c
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    //mbedtls_debug_set_threshold(4); // verbose tls handshake debugging
    //mbedtls_ssl_conf_dbg()
    //%%httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT(); //$$
    config.task_priority = 1U;
    //%%config.httpd.task_priority = 1U;
    ////config.server_port = 80;

    /*const char *servercert =
#include "certs/servercert.pem"
        ; //$$
    config.cacert_pem = (uint8_t*)servercert; //$$
    config.cacert_len = strlen(servercert); //$$*/
    
    //%extern const unsigned char servercert_start[] asm("_binary_include_certs_servercert_pem_start");
    //%extern const unsigned char servercert_end[] asm("_binary_include_certs_servercert_pem_end");
    //%config.cacert_pem = servercert_start;
    //%config.cacert_len = servercert_end - servercert_start;

    //%Serial.println("CACERT ASM LENGTH: " + String(config.cacert_len)); //DEBUG
    
    /*const char *prvtkey =
#include "certs/prvtkey.pem"
        ; //$$
    config.prvtkey_pem = (uint8_t *)prvtkey; //$$
    config.prvtkey_len = strlen(prvtkey); //$$*/
    //%extern const unsigned char prvtkey_start[] asm("_binary_include_certs_prvtkey_pem_start");
    //%extern const unsigned char prvtkey_end[] asm("_binary_include_certs_prvtkey_pem_end");
    //%config.prvtkey_pem = prvtkey_start;
    //%config.prvtkey_len = prvtkey_end - prvtkey_start;

    //MBEDTLS_ERR_SSL_WANT_READ
    //MBEDTLS_ERR_SSL_WANT_WRITE
    //MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS
    //MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS

    //%Serial.println("PRVTKEY ASM LENGTH: " + String(config.prvtkey_len)); //DEBUG

    httpd_uri_t index_uri_ota_upload = {
        .uri = "/ota_upload",
        .method = HTTP_GET,
        .handler = ota_upload_handler,
        .user_ctx = NULL
    };
    
    // Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&server_httpd, &config) == ESP_OK)
    //%if(httpd_ssl_start(&server_httpd, &config)) //$$
    {
        httpd_register_uri_handler(server_httpd, &index_uri_ota_upload);
    }
}