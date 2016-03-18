#ifndef __MAVLINK_GLUE_H__
#define __MAVLINK_GLUE_H__


// #ifndef MAVLINK_USE_CONVENIENCE_FUNCTIONS
// #define MAVLINK_USE_CONVENIENCE_FUNCTIONS 1
// #endif


#include "mavlink_types.h"

/**
 * @brief Send one char (uint8_t) over a comm channel
 *
 * @param chan MAVLink channel to use, usually MAVLINK_COMM_0 = UART0
 * @param ch Character to send
 */
// static inline void comm_send_ch(mavlink_channel_t chan, uint8_t ch)
// {
//     if (chan == MAVLINK_COMM_0)
//     {
//         Serial1.write(ch);
//     }
// }

//required by helper functions:
// mavlink_system_t mavlink_system;

#include "mavlink.h"

#endif //__MAVLINK_GLUE_H__

