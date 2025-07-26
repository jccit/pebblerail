import fs from "fs";

const pebbleColours = {
  GColorBlack: 0b11000000,
  GColorOxfordBlue: 0b11000001,
  GColorDukeBlue: 0b11000010,
  GColorBlue: 0b11000011,
  GColorDarkGreen: 0b11000100,
  GColorMidnightGreen: 0b11000101,
  GColorCobaltBlue: 0b11000110,
  GColorBlueMoon: 0b11000111,
  GColorIslamicGreen: 0b11001000,
  GColorJaegerGreen: 0b11001001,
  GColorTiffanyBlue: 0b11001010,
  GColorVividCerulean: 0b11001011,
  GColorGreen: 0b11001100,
  GColorMalachite: 0b11001101,
  GColorMediumSpringGreen: 0b11001110,
  GColorCyan: 0b11001111,
  GColorBulgarianRose: 0b11010000,
  GColorImperialPurple: 0b11010001,
  GColorIndigo: 0b11010010,
  GColorElectricUltramarine: 0b11010011,
  GColorArmyGreen: 0b11010100,
  GColorDarkGray: 0b11010101,
  GColorLiberty: 0b11010110,
  GColorVeryLightBlue: 0b11010111,
  GColorKellyGreen: 0b11011000,
  GColorMayGreen: 0b11011001,
  GColorCadetBlue: 0b11011010,
  GColorPictonBlue: 0b11011011,
  GColorBrightGreen: 0b11011100,
  GColorScreaminGreen: 0b11011101,
  GColorMediumAquamarine: 0b11011110,
  GColorElectricBlue: 0b11011111,
  GColorDarkCandyAppleRed: 0b11100000,
  GColorJazzberryJam: 0b11100001,
  GColorPurple: 0b11100010,
  GColorVividViolet: 0b11100011,
  GColorWindsorTan: 0b11100100,
  GColorRoseVale: 0b11100101,
  GColorPurpureus: 0b11100110,
  GColorLavenderIndigo: 0b11100111,
  GColorLimerick: 0b11101000,
  GColorBrass: 0b11101001,
  GColorLightGray: 0b11101010,
  GColorBabyBlueEyes: 0b11101011,
  GColorSpringBud: 0b11101100,
  GColorInchworm: 0b11101101,
  GColorMintGreen: 0b11101110,
  GColorCeleste: 0b11101111,
  GColorRed: 0b11110000,
  GColorFolly: 0b11110001,
  GColorFashionMagenta: 0b11110010,
  GColorMagenta: 0b11110011,
  GColorOrange: 0b11110100,
  GColorSunsetOrange: 0b11110101,
  GColorBrilliantRose: 0b11110110,
  GColorShockingPink: 0b11110111,
  GColorChromeYellow: 0b11111000,
  GColorRajah: 0b11111001,
  GColorMelon: 0b11111010,
  GColorRichBrilliantLavender: 0b11111011,
  GColorYellow: 0b11111100,
  GColorIcterine: 0b11111101,
  GColorPastelYellow: 0b11111110,
  GColorWhite: 0b11111111,
};

const operators = [
  { code: "AW", name: "Transport for Wales", colour: "GColorRed" },
  { code: "CC", name: "c2c", colour: "GColorPurple" },
  { code: "CH", name: "Chiltern Railways", colour: "GColorCyan" },
  { code: "CS", name: "Caledonian Sleeper", colour: "GColorMidnightGreen" },
  { code: "EM", name: "East Midlands Railway", colour: "GColorImperialPurple" },
  { code: "ES", name: "Eurostar", colour: "GColorDukeBlue" },
  { code: "GC", name: "Grand Central", colour: "GColorBlack" },
  { code: "GN", name: "Great Northern", colour: "GColorDarkCandyAppleRed" },
  { code: "GR", name: "London North Eastern Railway", colour: "GColorRed" },
  { code: "GW", name: "Great Western Railway", colour: "GColorDarkGreen" },
  { code: "GX", name: "Gatwick Express", colour: "GColorRed" },
  { code: "HC", name: "Heathrow Connect", colour: "GColorOrange" },
  { code: "HT", name: "Hull Trains", colour: "GColorBlue" },
  { code: "HX", name: "Heathrow Express", colour: "GColorIndigo" },
  { code: "IL", name: "Island Line", colour: "GColorCobaltBlue" },
  { code: "LD", name: "Lumo", colour: "GColorBlue" },
  { code: "LE", name: "Greater Anglia", colour: "GColorRed" },
  { code: "LM", name: "West Midlands Trains", colour: "GColorChromeYellow" },
  { code: "LO", name: "London Overground", colour: "GColorOrange" },
  { code: "LT", name: "London Underground", colour: "GColorRed" },
  { code: "ME", name: "Merseyrail", colour: "GColorYellow" },
  { code: "NT", name: "Northern", colour: "GColorDukeBlue" },
  { code: "NY", name: "North Yorkshire Moors Railway", colour: "GColorBlack" },
  { code: "PC", name: "Private Charter", colour: "GColorBlack" },
  { code: "RT", name: "Network Rail", colour: "GColorOrange" },
  { code: "SE", name: "Southeastern", colour: "GColorOxfordBlue" },
  { code: "SJ", name: "Sheffield Supertram", colour: "GColorDukeBlue" },
  { code: "SN", name: "Southern", colour: "GColorMayGreen" },
  { code: "SP", name: "Swanage Railway", colour: "GColorBlack" },
  { code: "SR", name: "ScotRail", colour: "GColorDukeBlue" },
  { code: "SW", name: "South Western Railway", colour: "GColorOxfordBlue" },
  { code: "TL", name: "Thameslink", colour: "GColorBrilliantRose" },
  { code: "TP", name: "TransPennine Express", colour: "GColorVividCerulean" },
  { code: "TW", name: "Tyne and Wear Metro", colour: "GColorYellow" },
  { code: "VT", name: "Avanti West Coast", colour: "GColorMidnightGreen" },
  { code: "WR", name: "West Coast Railway Co", colour: "GColorBulgarianRose" },
  { code: "XC", name: "CrossCountry", colour: "GColorJazzberryJam" },
  { code: "XR", name: "Elizabeth Line", colour: "GColorIndigo" },
  { code: "ZB", name: "Bus Station", colour: "GColorBlack" },
  { code: "ZF", name: "Ferry Terminal", colour: "GColorCobaltBlue" },
  { code: "ZM", name: "West Somerset Railway", colour: "GColorBlack" },
];

/*
operators.bin format:
name = 32 bytes
colour = 1 byte
----------------
stride = 33 bytes

the pebble app uses an index to lookup the operator info
by multiplying the index by 33 to find the start, then reading the next 33 bytes
*/

const file = fs.openSync(new URL("operators.bin", import.meta.url), "w");

for (const operator of operators) {
  const name = operator.name.slice(0, 32).padEnd(32, "\0");
  fs.writeSync(file, name);

  const colour = Buffer.from([pebbleColours[operator.colour]]);
  fs.writeSync(file, colour);
}

fs.closeSync(file);
