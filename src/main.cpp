/*
 * Author: Pipisara Chandrabhanu
 * Date: [2024-09-10]
 *
 * This code implements a web server on the ESP32 to stream live video from an OV2640 camera module.
 * The server allows users to:
 *   - View a live JPEG stream at: http://[ESP32_IP]/live
 *   - Capture individual JPEG images at: http://[ESP32_IP]/jpg
 *   - Turn the camera's flash ON at: http://[ESP32_IP]/on
 *   - Turn the camera's flash OFF at: http://[ESP32_IP]/off
 * WiFi is used to establish the connection, and the onboard LED provides visual feedback for connection status.
 */




#include "OV2640.h"  // Include camera library for OV2640 module
#include <WiFi.h>        // Include WiFi library for ESP32
#include <WebServer.h>   // Include web server library for serving web requests
#include <WiFiClient.h>  // Include WiFi client library for handling client connections
#include <Arduino.h>


// WiFi credentials
// const char* ssid = "OMENHE";  // Replace with your WiFi SSID
// const char* password = "818Y95g0";    // Replace with your WiFi password

const char* ssid = "realmeGTHe";  // Replace with your WiFi SSID
const char* password = "12345678";    // Replace with your WiFi password

//修改为手机热点名称后，在手机浏览器输入对应网址观察信息


OV2640 cam;              // Initialize the camera object
WebServer server(80);    // Create a web server object on port 80

// HTTP response headers for streaming JPEG images
const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);  // Calculate the length of the header
const int bdrLen = strlen(BOUNDARY);  // Calculate the length of the boundary
const int cntLen = strlen(CTNTTYPE);  // Calculate the length of the content type string

bool wifiConnected = false;  // Flag to track WiFi connection status
unsigned long previousMillis = 0;  // Timer variable for LED blinking
const long interval = 500;    // Interval for blinking LED (500ms)

// Handle streaming of JPEG images
void handle_jpg_stream(void)
{
  char buf[32];  // Buffer to store the size of the image
  int s;

  WiFiClient client = server.client();  // Get the connected client

  client.write(HEADER, hdrLen);  // Send the header to the client
  client.write(BOUNDARY, bdrLen);  // Send the boundary to the client

  while (true)
  {
    if (!client.connected()) break;  // Stop if the client disconnects
    cam.run();  // Capture an image from the camera
    s = cam.getSize();  // Get the size of the image
    client.write(CTNTTYPE, cntLen);  // Send the content type to the client
    sprintf(buf, "%d\r\n\r\n", s);  // Format the size of the image
    client.write(buf, strlen(buf));  // Send the image size to the client
    client.write((char *)cam.getfb(), s);  // Send the image data to the client
    client.write(BOUNDARY, bdrLen);  // Send the boundary to the client
  }
}

// HTTP response header for serving a single JPEG image
const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);  // Calculate the length of the header

// Handle serving a single JPEG image
void handle_jpg(void)
{
  WiFiClient client = server.client();  // Get the connected client

  cam.run();  // Capture an image from the camera
  if (!client.connected()) return;  // Stop if the client disconnects

  client.write(JHEADER, jhdLen);  // Send the header to the client
  client.write((char *)cam.getfb(), cam.getSize());  // Send the image data to the client
}


// Handle requests for undefined routes
void handleNotFound()
{
  String message = "Server is running!\n\n";  // Message to send when an undefined route is accessed
  message += "URI: ";  // Add the requested URI to the message
  message += server.uri();
  message += "\nMethod: ";  // Add the HTTP method to the message
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";  // Add the number of arguments to the message
  message += server.args();
  message += "\n";
  server.send(200, "text/plain", message);  // Send the message to the client
}

// Setup function to initialize the camera, WiFi, and web server
void setup()
{
  Serial.begin(115200);  // Start the serial communication at 115200 baud rate
  Serial.println("ESP32-S3 Camera Test");
  
  // Camera configuration
  camera_config_t config;

  config.pin_d0 = 40;
  config.pin_d1 = 42;
  config.pin_d2 = 41;
  config.pin_d3 = 2;
  config.pin_d4 = 6;
  config.pin_d5 = 1;
  config.pin_d6 = 5;
  config.pin_d7 = 14;//
  config.pin_xclk = -1;
  config.pin_pclk = 39;//
  config.pin_vsync =11;
  config.pin_href = 9;
  config.pin_sccb_sda = 10;
  config.pin_sccb_scl = 21;
  config.pin_pwdn = -1;
  config.pin_reset =-1;

  // 时钟和像素格式配置
  config.xclk_freq_hz = 20000000; // XCLK 频率 20MHz
  config.ledc_timer = LEDC_TIMER_0;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.pixel_format = PIXFORMAT_JPEG; // 输出格式为JPEG
  config.fb_location = CAMERA_FB_IN_PSRAM; // **关键**: ESP32-S3必须使用PSRAM作为帧缓冲区
  config.frame_size = FRAMESIZE_VGA; // , S3的PSRAM可以轻松支持更高分辨率
  config.jpeg_quality = 12; // 0-63, 数字越小质量越高
  config.fb_count = 2; // 使用2个帧缓冲区，可以提高流传输的流畅度
  config.grab_mode = CAMERA_GRAB_LATEST; // 获取最新的帧

  // 初始化摄像头
  esp_err_t err = cam.init(config);
  if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      return;
  }
  Serial.println("Camera init OK");

  // Initialize WiFi in station mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  // Connect to the WiFi network using credentials
}

// Main loop function to handle WiFi connection and client requests
void loop()
{
  unsigned long currentMillis = millis();  // Get the current time

  // If not connected to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    if (currentMillis - previousMillis >= interval) {  // Check if interval has passed
      previousMillis = currentMillis;
    }
  } else if (!wifiConnected) {
    wifiConnected = true;  // Mark WiFi as connected

    // Print connection details to the serial monitor
    Serial.println(F("WiFi connected"));
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
    Serial.print("Stream Link: http://");
    Serial.print(ip);
    Serial.println("/live");

    // Print flash control URLs to the serial monitor
    //Serial.print("Flash ON URL: http://");
    Serial.print(ip);
    Serial.println("/on");

    //Serial.print("Flash OFF URL: http://");
    Serial.print(ip);
    Serial.println("/off");

    // Define server routes
    server.on("/live", HTTP_GET, handle_jpg_stream);  // Route for live stream
    server.on("/jpg", HTTP_GET, handle_jpg);          // Route for single image capture
    server.onNotFound(handleNotFound);                // Route for undefined routes

    server.begin();  // Start the server
  }

  server.handleClient();  // Handle incoming client requests
}
