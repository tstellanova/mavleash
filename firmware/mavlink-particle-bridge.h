#ifndef _PARTICLE_MAVLINK_LIB
#define _PARTICLE_MAVLINK_LIB

// Make library cross-compatiable
// with Arduino, GNU C++ for tests, and Spark.
//#if defined(ARDUINO) && ARDUINO >= 100
//#include "Arduino.h"
//#elif defined(SPARK)
//#include "application.h"
//#endif

// TEMPORARY UNTIL the stuff that supports the code above is deployed to the build IDE
#include "application.h"

#include "mavlink_glue.h"


namespace ParticleMavlinkLibrary 
{
  //Manages a mavlink connection via Serial
  class MavlinkBridge
  {
  protected:
    mavlink_message_t m_msg;
    mavlink_status_t m_status;
    bool m_mavlink_available;///< Have we parsed at least one mavlink msg successfully?

    uint8_t m_mavlink_sysid;///< The mavlink system ID of the autopilot we're communicating with
    uint8_t m_mavlink_compid;///< The mavlink component ID of the autopilot we're communicating with
    
  public:
    MavlinkBridge() {};
    void init();//TODO make Serial port and baud rate configurable
    bool readMavlinkMsg(mavlink_message_t& msg);
    bool handleCommand(String params);
    
  protected:
    bool sendCommandRTL();
    bool sendCommandLand();
    bool sendCommandTakeoff();
    bool sendLongCommand(uint16_t command);

  };
}

#endif
