#include "config.h"
#include "ax25.h"
#include "gps.h"
#include "aprs.h"
#include <stdio.h>
#include <stdlib.h>
#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif

// Exported functions
void aprs_send()
{
  char temp[12];                   // Temperature (int/ext)
  const struct s_address addresses[] = { 
    {D_CALLSIGN, D_CALLSIGN_ID},  // Destination callsign
    {S_CALLSIGN, S_CALLSIGN_ID},  // Source callsign (-11 = balloon, -9 = car)
#ifdef DIGI_PATH1
    {DIGI_PATH1, DIGI_PATH1_TTL}, // Digi1 (first digi in the chain)
#endif
#ifdef DIGI_PATH2
    {DIGI_PATH2, DIGI_PATH2_TTL}, // Digi2 (second digi in the chain)
#endif
  };

  ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address));
  ax25_send_byte('/');                // Report w/ timestamp, no APRS messaging. $ = NMEA raw data
  ax25_send_string(gps_time);         // 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
  ax25_send_byte('h');
  ax25_send_string(gps_aprs_lat);     // Lat: 38deg and 22.20 min (.20 are NOT seconds, but 1/100th of minutes)
  ax25_send_byte('/');                // Symbol table
  ax25_send_string(gps_aprs_lon);     // Lon: 000deg and 25.80 min
  ax25_send_byte('>');                // Symbol: O=balloon, -=QTH
  snprintf(temp, 4, "%03d", (int) gps_course); 
  ax25_send_string(temp);             // Course (degrees)
  ax25_send_byte('/');                // and
  snprintf(temp, 4, "%03d", (int) (gps_speed / 1.151));
  ax25_send_string(temp);             // speed (knots)
  ax25_send_string("/A=");            // Altitude (feet). Goes anywhere in the comment area
  snprintf(temp, 7, "%06d", (int) (gps_altitude / 0.3048));
  ax25_send_string(temp);
  //ax25_send_byte(' ');
  //ax25_send_string(APRS_COMMENT);     // Comment
  ax25_send_footer();

  ax25_flush_frame();                 // Tell the modem to go
}
