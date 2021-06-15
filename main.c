/***********************************
*             main.c
*
* -Main program: controls all modbus
*  queries and data exchange.
*
***********************************/
// Include the modbus header
#include "modbus.h"
#include "mqttClient.h"

#define COM "/dev/ttyUSB0"   // For MODBUS device: "ttyUSB0"

// Internal Functions
void publishMsgs();

// Internal Variables
uint8_t M_ID = 0x01;

int main(void){
    // Setup MQTT
    mqtt_setup();

    // Try until port is connected
    while(1){
        printf("** Connecting to USB Port\n");
        // Initialize port
        if(initializePort(COM) < 0) continue;
        else break;
        // Wait 5 seconds
        usleep(5*1000*1000);
    }

    // Infinite loop for publishing to MQTT
    while(1){
        printf("**--Begining Publishing Loop--**\n");
        // Wait to not overload the system
        if(usleep(getScanRate()*1000) < 0) printf("ERROR %i from usleep: %s\n", errno, strerror(errno));
        // Publish all the parameters to the MQTT Broker
        printf("**Publishing to MQTT\n");
        publishMsgs();
        modbusQuery(M_ID,VOLTAGE);
    }
}

void publishMsgs(){
    char s[64];
    
    sprintf(s,"%f",modbusQuery(M_ID,VOLTAGE));
    mqtt_send(s,"parameters/voltage");
    sprintf(s,"%f",modbusQuery(M_ID, CURRENT));
    mqtt_send(s,"parameters/current");
    sprintf(s,"%f",modbusQuery(M_ID, POWER_FACTOR));
    mqtt_send(s,"parameters/pf");
    sprintf(s,"%f",modbusQuery(M_ID, PHASE));
    mqtt_send(s,"parameters/phase");
    sprintf(s,"%f",modbusQuery(M_ID, FREQUENCY));
    mqtt_send(s,"parameters/frequency");

    sprintf(s,"%f",modbusQuery(M_ID, ACT_ENERGY_IM));
    mqtt_send(s,"energy/import/active");
    sprintf(s,"%f",modbusQuery(M_ID, REA_ENERGY_IM));
    mqtt_send(s,"energy/import/reactive");
    sprintf(s,"%f",modbusQuery(M_ID, ACT_ENERGY_EX));
    mqtt_send(s,"energy/export/active");
    sprintf(s,"%f",modbusQuery(M_ID, REA_ENERGY_EX));
    mqtt_send(s,"energy/export/reactive");

    sprintf(s,"%f",modbusQuery(M_ID, POWER_APPARENT));
    mqtt_send(s,"power/apparent");
    sprintf(s,"%f",modbusQuery(M_ID, POWER_ACTIVE));
    mqtt_send(s,"power/active");
    sprintf(s,"%f",modbusQuery(M_ID, POWER_REACTIVE));
    mqtt_send(s,"power/reactive");
}

