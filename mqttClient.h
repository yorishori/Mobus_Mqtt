#ifndef mqttClient
#define mqttClient

// Standard C libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// Include MQTT Library
#include <mosquitto.h>

// Shared Variables


// Shared functions
void mqtt_setup();
int mqtt_send(char *msg, char *topic);
int getScanRate();

#endif