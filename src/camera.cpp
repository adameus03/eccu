#include "Arduino.h"
#include "esp_camera.h" //OV2640

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
  
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

camera_fb_t *fb = NULL;

esp_err_t initialize_camera(){
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG/*PIXFORMAT_RGB565*/;

    //config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    //config.grab_mode = CAMERA_GRAB_LATEST;
    //config.fb_location = CAMERA_FB_IN_PSRAM;
    //config.fb_location = CAMERA_FB_IN_DRAM;


    config.frame_size = FRAMESIZE_HVGA /*FRAMESIZE_QVGA*/ /*FRAMESIZE_SVGA*/ /*FRAMESIZE_UXGA*/;
    config.jpeg_quality = 10/*12*//*20*/;
    config.fb_count = 2; // what about more?

    esp_err_t res = esp_camera_init(&config);
    if(res == ESP_OK){
        sensor_t* s = esp_camera_sensor_get();
        s->set_raw_gma(s, 1);
        s->set_denoise(s, 1); //?
        s->set_whitebal(s, 1);
        s->set_exposure_ctrl(s, 1);
        s->set_lenc(s, 1);
        Serial.println("OV2640 sensor configured");
    }
    return res;

}

esp_err_t reinitialize_camera(){

}

void freeze_load_fb(){
    fb = esp_camera_fb_get();
    if(!fb){
        Serial.println("NULL detected in freeze_load_fb()");
        //Serial.println("Reinitializing camera (driver)...");
        //esp_camera_deinit();
        //initialize_camera();
    }
}

void release_fb(){
    esp_camera_fb_return(fb);
    //(fb->buf); //experimental
    //fb = NULL; //maybe this is the cause of the memory leak, because esp_camera_fb_return(fb) would still be trying to free the buffer, but fb is already NULL?
}

camera_fb_t* get_fb_no_reserve(){
    return fb;
}