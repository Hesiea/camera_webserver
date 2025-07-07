/*
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
// const char* ssid = "HONOR";  // Replace with your WiFi SSID
// const char* password = "11111111";    // Replace with your WiFi password

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
  
  // 初始化摄像头
  esp_err_t err = cam.init(esp32s3_devkitc1_config);
  
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
