/*
 * This code implements a web server on the ESP32 to stream live video from an OV2640 camera module.
 * The server allows users to view a live JPEG stream, capture individual JPEG images, and control 
 * the camera's flash LED remotely via a web interface. WiFi is used to establish the connection, 
 * and the onboard LED provides visual feedback for connection status.
 */



#ifndef OV2640_H_
#define OV2640_H_

#include <Arduino.h>
#include <pgmspace.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_camera.h"

extern camera_config_t esp32cam_config, esp32cam_aithinker_config, esp32cam_ttgo_t_config,esp32s3_devkitc1_config;

class OV2640
{
public:
    OV2640(){
        fb = NULL;
    };
    ~OV2640(){
    };
    esp_err_t init(camera_config_t config);
    void run(void);
    size_t getSize(void);
    uint8_t *getfb(void);
    int getWidth(void);
    int getHeight(void);
    framesize_t getFrameSize(void);
    pixformat_t getPixelFormat(void);

    void setFrameSize(framesize_t size);
    void setPixelFormat(pixformat_t format);

private:
    void runIfNeeded(); // grab a frame if we don't already have one

    // camera_framesize_t _frame_size;
    // camera_pixelformat_t _pixel_format;
    camera_config_t _cam_config;

    camera_fb_t *fb;
};

#endif //OV2640_H_
