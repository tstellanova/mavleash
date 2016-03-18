// IMPORTANT: When including a library in a firmware app, a sub dir prefix is needed
// before the particular .h file.
#include "mavlink-particle-bridge.h"

// Initialize objects from the lib; be sure not to call anything
// that requires hardware be initialized here, put those in setup()
ParticleMavlinkLibrary::MavlinkBridge mavBridge;


double	latitude_degrees = 0;
double	longitude_degrees = 0;
double  altitude_amsl_m = 0; //altitude AMSL, meters
double  groundspeed_m_s = 0; //groundspeed in meters per second
double  heading_degrees = 0; //heading in degrees, 0 = North
double  battery_volts = 0;//remaining battery potential, 0.0 if no battery present

int command_LED = D7;

// vehicle landed state (MAV_LANDED_STATE)
int landed_state = MAV_LANDED_STATE_UNDEFINED;

uint32_t  last_msg = 0;
String last_cmd = "--";
int heartbeat_count = 0;//rate-limit transmissions OTA

int handleCommand(String params)  {
  int result = -1;
  
  if (params == "Reset") {
    System.reset();
    return 0; 
  }

  digitalWrite(command_LED, HIGH);

  bool success = mavBridge.handleCommand(params);
  if (success) {
    result = 0;
    last_cmd = params;
  }
  
  return result;
}

/**
Publish a summary of the MAV state in JSON format, a more compact format than individual Particle.variables:
this reduces data usage (with Electron especially)
*/
void publishStateAsJSON() {

  if (!((latitude_degrees == 0) && (longitude_degrees == 0))) {
    Particle.publish("statejson", 
      String::format("{\"lat\":%0.5f, \"lon\":%0.5f, \"amsl\":%0.2f, \"speed\":%0.2f, \"head\":%0.2f, \"volts\":%0.2f, \"cmd\":\"%s\"}",
        latitude_degrees,
        longitude_degrees,
        altitude_amsl_m,
        groundspeed_m_s,
        heading_degrees,
        battery_volts,
        last_cmd.c_str()
        ),
        30);
    }
}

/**
Publish a summary of the MAV state in CSV, an even more compact form than JSON
*/
void publishStateAsCSV() {

  if (!((latitude_degrees == 0) && (longitude_degrees == 0))) {
    Particle.publish("statecsv", 
      String::format("[%0.5f,%0.5f,%0.2f,%0.2f,%0.2f,%0.2f,\"%s\"]",
      latitude_degrees,
      longitude_degrees,
      altitude_amsl_m,
      groundspeed_m_s,
      heading_degrees,
      battery_volts,
      last_cmd.c_str()
      ),
      30);
  }
}

/**
* Publish an event when the MAV_LANDED_STATE changes
*/
void publishLandedState() {
  switch (landed_state) {
  case MAV_LANDED_STATE_ON_GROUND:
    Particle.publish("LANDED_STATE","ON_GROUND");
    break;
  case MAV_LANDED_STATE_IN_AIR:
    Particle.publish("LANDED_STATE","IN_AIR");
    break;              
  }
}

/**
* Register some cloud variables to be monitored
*/
// void registerCloudVariables() {
//   // note that variable names are limited to 12 characters currently
//   Particle.variable("latitude", latitude_degrees);
//   Particle.variable("longitude", longitude_degrees);
//   Particle.variable("altitude", altitude_amsl_m);
//   Particle.variable("groundspeed",groundspeed_m_s);
//   Particle.variable("heading",heading_degrees);
//   Particle.variable("battery_v",battery_volts);
//   Particle.variable("last_msg",last_msg);
//   Particle.variable("last_cmd", last_cmd);
// }
  
void setup() {
  // Call functions on initialized library objects that require hardware
  // to be wired and available.
  mavBridge.init();

  // Register cloud variables (if using these)
  //registerCloudVariables();

  // register some remote commands:
  // note that function names are limited to 12 characters currently
  Particle.function("command", handleCommand);
  
  //setup for indicating command received
  pinMode(command_LED, OUTPUT);
  digitalWrite(command_LED, LOW);


}

void handleMavlinkMsg(const mavlink_message_t& msg)
{
    last_msg = msg.msgid;
    switch(msg.msgid) {
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
          //Latitude, expressed as degrees * 1E7
          latitude_degrees = ((double)mavlink_msg_global_position_int_get_lat(&msg)) / 1E7;
          longitude_degrees = ((double)mavlink_msg_global_position_int_get_lon(&msg)) / 1E7;

          //Altitude in meters, expressed as * 1000 (millimeters), AMSL (not WGS84)
          altitude_amsl_m = ((double)mavlink_msg_global_position_int_get_alt(&msg))/1000.0f;

          //Altitude above ground in meters, expressed as * 1000 (millimeters)
          //tends to be inaccurate unless we have an AGL sensor
          //mavlink_msg_global_position_int_get_relative_alt(&msg);

          //Vehicle heading (yaw angle) in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
          heading_degrees = mavlink_msg_global_position_int_get_hdg(&msg) / 100.0f;
          
          //TODO: NED velocity reporting?
          //mavlink_msg_global_position_int_get_vx
        }
        break;
        case MAVLINK_MSG_ID_SYS_STATUS: {
          //obtain the battery voltage from the reading in millivolts
          battery_volts = mavlink_msg_sys_status_get_voltage_battery(&msg) / 1000.0f;
        }
        break;
        case MAVLINK_MSG_ID_VFR_HUD: {
          //Current ground speed in m/s
          groundspeed_m_s = mavlink_msg_vfr_hud_get_groundspeed(&msg);
          //Current heading in degrees, in compass units (0..360, 0=north)
          //this is less precise than the same value from GLOBAL_POSITION_INT
          //heading_degrees = mavlink_msg_vfr_hud_get_heading(&msg)
        }
        break;
        case MAVLINK_MSG_ID_HEARTBEAT: {
          //TODO: handle mavlink_msg_heartbeat_get_base_mode and so forth
          //We know that this message arrives at about 1Hz, which gives a nice base update rate for publication
          heartbeat_count++;
          if (heartbeat_count > 3) { //rate limit transmissions
            publishStateAsCSV();
            heartbeat_count = 0;
          }  
        }
        break;
        case MAVLINK_MSG_ID_EXTENDED_SYS_STATE: {
          uint8_t new_landed_state =  mavlink_msg_extended_sys_state_get_landed_state(&msg);
          if (new_landed_state != landed_state) {
            landed_state = new_landed_state;
            publishLandedState();
          }
        }
        break;
    }
}


void loop() {
    mavlink_message_t msg;
    if (mavBridge.readMavlinkMsg(msg)) {
        //received one mavlink message
        handleMavlinkMsg(msg);
    }
    
    //give some CPU cycles to Particle housekeeping
    Particle.process();
}