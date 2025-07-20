#include "tocs.h"

#ifdef PBL_BW
#define OPERATOR_INFO(code, name, color)      \
  if (strcmp(requested_code, code) == 0) {    \
    return (OperatorInfo){name, GColorWhite}; \
  }
#else
#define OPERATOR_INFO(code, name, color)   \
  if (strcmp(requested_code, code) == 0) { \
    return (OperatorInfo){name, color};    \
  }
#endif

OperatorInfo operator_info(char *requested_code) {
  OPERATOR_INFO("AW", "Transport for Wales", GColorRed);
  OPERATOR_INFO("CC", "c2c", GColorPurple);
  OPERATOR_INFO("CH", "Chiltern Railways", GColorCyan);
  OPERATOR_INFO("CS", "Caledonian Sleeper", GColorMidnightGreen);
  OPERATOR_INFO("EM", "East Midlands Railway", GColorImperialPurple);
  OPERATOR_INFO("ES", "Eurostar", GColorDukeBlue);
  OPERATOR_INFO("GC", "Grand Central", GColorBlack);
  OPERATOR_INFO("GN", "Great Northern", GColorDarkCandyAppleRed);
  OPERATOR_INFO("GR", "London North Eastern Railway", GColorRed);
  OPERATOR_INFO("GW", "Great Western Railway", GColorDarkGreen);
  OPERATOR_INFO("GX", "Gatwick Express", GColorRed);
  OPERATOR_INFO("HC", "Heathrow Connect", GColorOrange);
  OPERATOR_INFO("HT", "Hull Trains", GColorBlue);
  OPERATOR_INFO("HX", "Heathrow Express", GColorIndigo);
  OPERATOR_INFO("IL", "Island Line", GColorCobaltBlue);
  OPERATOR_INFO("LD", "Lumo", GColorBlue);
  OPERATOR_INFO("LE", "Greater Anglia", GColorRed);
  OPERATOR_INFO("LM", "West Midlands Trains", GColorChromeYellow);
  OPERATOR_INFO("LO", "London Overground", GColorOrange);
  OPERATOR_INFO("LT", "London Underground", GColorRed);
  OPERATOR_INFO("ME", "Merseyrail", GColorYellow);
  OPERATOR_INFO("NT", "Northern", GColorDukeBlue);
  OPERATOR_INFO("NY", "North Yorkshire Moors Railway", GColorBlack);
  OPERATOR_INFO("PC", "Private Charter", GColorBlack);
  OPERATOR_INFO("RT", "Network Rail", GColorOrange);
  OPERATOR_INFO("SE", "Southeastern", GColorOxfordBlue);
  OPERATOR_INFO("SJ", "Sheffield Supertram", GColorDukeBlue);
  OPERATOR_INFO("SN", "Southern", GColorMayGreen);
  OPERATOR_INFO("SP", "Swanage Railway", GColorBlack);
  OPERATOR_INFO("SR", "ScotRail", GColorDukeBlue);
  OPERATOR_INFO("SW", "South Western Railway", GColorOxfordBlue);
  OPERATOR_INFO("TL", "Thameslink", GColorBrilliantRose);
  OPERATOR_INFO("TP", "TransPennine Express", GColorVividCerulean);
  OPERATOR_INFO("TW", "Tyne and Wear Metro", GColorYellow);
  OPERATOR_INFO("VT", "Avanti West Coast", GColorMidnightGreen);
  OPERATOR_INFO("WR", "West Coast Railway Co", GColorBulgarianRose);
  OPERATOR_INFO("XC", "CrossCountry", GColorJazzberryJam);
  OPERATOR_INFO("XR", "Elizabeth Line", GColorIndigo);
  OPERATOR_INFO("ZB", "Bus Station", GColorBlack);
  OPERATOR_INFO("ZF", "Ferry Terminal", GColorCobaltBlue);
  OPERATOR_INFO("ZM", "West Somerset Railway", GColorBlack);

  return (OperatorInfo){"Unknown", GColorBlack};
}
