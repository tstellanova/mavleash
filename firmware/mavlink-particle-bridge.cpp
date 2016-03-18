// NOTE/NUANCE: When including WITHIN a library, no sub-dir prefix is needed.
#include "mavlink-particle-bridge.h"

namespace ParticleMavlinkLibrary {


void MavlinkBridge::init()
{
  // open serial over TX and RX pins on Particle
  Serial1.begin(57600);  
  while (!Serial1.available()) {
    Particle.process();
  }

}

bool MavlinkBridge::readMavlinkMsg(mavlink_message_t& msg)
{
  while(Serial1.available() > 0) { 
    uint8_t c = Serial1.read();

    //hand the next byte to the mavlink parser, try to find a complete msg
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &m_msg, &m_status)) {
      m_mavlink_available = true;
      msg = m_msg;
      //ensure that we track the system's IDs for sending commands
      m_mavlink_sysid = msg.sysid;
      m_mavlink_compid = msg.compid;
      return true;
    }
    //TODO look at m_status ?
  }

  return false;
}

bool MavlinkBridge::handleCommand(String params)
{
  if (params == "RTL") {
    return sendCommandRTL();
  }
  else if (params == "Land") {
    return sendCommandLand();
  }
  else if (params == "Takeoff") {
    return sendCommandTakeoff();
  }
  
  return false;
}


bool MavlinkBridge::sendLongCommand(uint16_t command)
{
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  
  mavlink_msg_command_long_pack(255, 0, &msg,
    m_mavlink_sysid, 
    m_mavlink_compid, 
    command, 
    0, //uint8_t confirmation, 
    0, //float param1, 
    0, //float param2, 
    0, //float param3, 
    0, //float param4, 
    0, //float param5, 
    0, //float param6, 
    0);  //float param7);
    
  // Copy the message to the send buffer
  uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  int bytes_sent = Serial1.write(buf, len);
  
  return (bytes_sent == len);
}

bool MavlinkBridge::sendCommandRTL()
{
  //This might work with eg Arducopter, haven't tried it:
  return sendLongCommand(MAV_CMD_NAV_RETURN_TO_LAUNCH);
  //please refer to the individual autopilot specifications for details.
  
  // I tried the following on 20160318 with the PX4 multicopter sw stack,
  // but this is apparently not sufficient-- it appears you need to switch
  // into AUTO mode with a mission already active?
  /*
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  
  //set PX4_CUSTOM_SUB_MODE_AUTO_RTL for Pixhawk flight stack
  
  mavlink_msg_command_long_pack(255, 0, &msg,
    m_mavlink_sysid, 
    m_mavlink_compid, 
    MAV_CMD_DO_SET_MODE, 
    0, //uint8_t confirmation, 
    MAV_MODE_AUTO_ARMED, //float param1,  //Mission Param #1	Mode, as defined by ENUM MAV_MODE
    4, //PX4_CUSTOM_MAIN_MODE_AUTO, //float param2, //Mission Param #2	Custom mode - this is system specific
    5, //PX4_CUSTOM_SUB_MODE_AUTO_RTL, //float param3, //Mission Param #3	Custom sub mode - this is system specific
    0, //float param4, 
    0, //float param5, 
    0, //float param6, 
    0);  //float param7);
    
  // Copy the message to the send buffer
  uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  int bytes_sent = Serial1.write(buf, len);
  
  return (bytes_sent == len);
  */
}


bool MavlinkBridge::sendCommandLand()
{
  //tested 20160318 with PX4 multicopter in POSCTL mode
  return sendLongCommand(MAV_CMD_NAV_LAND);
}

bool MavlinkBridge::sendCommandTakeoff()
{
  //untested, but may work if the vehicle is already in AUTO mode and has a defined mission
  return sendLongCommand(MAV_CMD_NAV_TAKEOFF);
}


}


