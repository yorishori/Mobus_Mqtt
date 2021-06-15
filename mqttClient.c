/***********************************
*             mosquitto.c
*
* -MQTT Client Function Manager:
*  mainly starting the client and 
*  other functions.
*
***********************************/

// Include header
#include "mqttClient.h"

// Define constants
#define host "localhost"
#define port 1883
#define sub_topic "adqTime/"

// Internal Function
void log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str);
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

// Varibles
static struct mosquitto *mosq;
static int scanRate = 1000;        // Scanning rate in miliseconds

// Initializer of the MQTT Client
void mqtt_setup(){
    int keepalive = 60;
    bool clean_session = true;
    
    // Initialize Library
    mosquitto_lib_init();
    // Instantiate Client
    mosq = mosquitto_new(NULL, clean_session, NULL);
    if(!mosq){
	    fprintf(stderr, "Error: Out of memory.\n");
		exit(1);
	}

    // Set callbacks
    mosquitto_message_callback_set(mosq, message_callback);

    // Connect to MQTT Server
    if(mosquitto_connect(mosq, host, port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		exit(1);
	}

    // Subscribre to Adquisition Time topic "adqTime/""
    mosquitto_subscribe(mosq, NULL, sub_topic, 0);

    // Initialize callbacks
    int loop = mosquitto_loop_start(mosq);
    if(loop != MOSQ_ERR_SUCCESS){
        fprintf(stderr, "Unable to start loop: %i\n", loop);
        exit(1);
    }
}

// Callback for the logger
int mqtt_send(char *msg, char *topic){
    return mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, 2, true);
}

// Callback for the message
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){
	bool match = 0;
	printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

	mosquitto_topic_matches_sub(sub_topic, message->topic, &match);
	if (match) {
        scanRate = atoi((char*)message->payload);
		printf("Changed Scan Rate to: %i\n", scanRate);
        mqtt_send("Scan rate changed...","admin/");
	}
}

// Get scan rate
int getScanRate(){
    return scanRate;
}