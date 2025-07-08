#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>

// 初始化MQTT客户端，并连接到服务器
void mqtt_init(const char* server_ip);

// 在主循环中必须调用的函数，用于保持连接和接收消息
void mqtt_loop();

// 发布消息到指定主题
void mqtt_publish(const char* topic, const char* payload);

// 订阅一个主题
void mqtt_subscribe(const char* topic);

#endif // MQTT_MANAGER_H