#include "mqtt_manager.h"
#include <WiFi.h>
#include <PubSubClient.h>

// 全局对象
WiFiClient espClient;
PubSubClient client(espClient);
char mqtt_server_ip[20];
long lastReconnectAttempt = 0;

// 当收到订阅的消息时，这个回调函数会被触发
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // 在这里可以添加处理接收到消息的逻辑
    // 例如：如果topic是“/pillbox/speak”，就调用语音模块播报payload
}

// 重新连接函数
boolean reconnect() {
    if (client.connect("esp32-pillbox-client")) {
        Serial.println("MQTT Connected!");
        // 在这里可以重新订阅主题
        mqtt_subscribe("/pillbox/command");
    }
    return client.connected();
}

// ---- 公共函数的实现 ----

void mqtt_init(const char* server_ip) {
    strncpy(mqtt_server_ip, server_ip, sizeof(mqtt_server_ip));
    client.setServer(mqtt_server_ip, 1883);
    client.setCallback(callback);
}

void mqtt_loop() {
    if (!client.connected()) {
        long now = millis();
        // 每隔5秒尝试重连一次，避免阻塞
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            if (reconnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        // 保持客户端循环
        client.loop();
    }
}

void mqtt_publish(const char* topic, const char* payload) {
    if (client.connected()) {
        client.publish(topic, payload);
        Serial.printf("Published to topic %s: %s\n", topic, payload);
    } else {
        Serial.println("MQTT client not connected. Cannot publish.");
    }
}

void mqtt_subscribe(const char* topic) {
    if (client.connected()) {
        client.subscribe(topic);
        Serial.printf("Subscribed to topic: %s\n", topic);
    }
}