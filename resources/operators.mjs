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
  {
    code: "AW",
    name: "Transport for Wales",
    colour: "GColorRed",
    metro: true,
  },
  {
    code: "CC",
    name: "c2c",
    colour: "GColorPurple",
    metro: false,
  },
  {
    code: "CH",
    name: "Chiltern Railways",
    colour: "GColorCyan",
    metro: true,
  },
  {
    code: "CS",
    name: "Caledonian Sleeper",
    colour: "GColorMidnightGreen",
    metro: false,
  },
  {
    code: "EM",
    name: "East Midlands Railway",
    colour: "GColorImperialPurple",
    metro: false,
  },
  {
    code: "ES",
    name: "Eurostar",
    colour: "GColorDukeBlue",
    metro: false,
  },
  {
    code: "GC",
    name: "Grand Central",
    colour: "GColorBlack",
    metro: false,
  },
  {
    code: "GN",
    name: "Great Northern",
    colour: "GColorDarkCandyAppleRed",
    metro: true,
  },
  {
    code: "GR",
    name: "London North Eastern Railway",
    colour: "GColorRed",
    metro: false,
  },
  {
    code: "GW",
    name: "Great Western Railway",
    colour: "GColorDarkGreen",
    metro: false,
  },
  {
    code: "GX",
    name: "Gatwick Express",
    colour: "GColorRed",
    metro: true,
  },
  {
    code: "HC",
    name: "Heathrow Connect",
    colour: "GColorOrange",
    metro: true,
  },
  {
    code: "HT",
    name: "Hull Trains",
    colour: "GColorBlue",
    metro: false,
  },
  {
    code: "HX",
    name: "Heathrow Express",
    colour: "GColorIndigo",
    metro: true,
  },
  {
    code: "IL",
    name: "Island Line",
    colour: "GColorCobaltBlue",
    metro: true,
  },
  {
    code: "LD",
    name: "Lumo",
    colour: "GColorBlue",
    metro: false,
  },
  {
    code: "LE",
    name: "Greater Anglia",
    colour: "GColorRed",
    metro: false,
  },
  {
    code: "LM",
    name: "West Midlands Trains",
    colour: "GColorChromeYellow",
    metro: false,
  },
  {
    code: "LO",
    name: "London Overground",
    colour: "GColorOrange",
    metro: true,
  },
  {
    code: "LT",
    name: "London Underground",
    colour: "GColorRed",
    metro: true,
  },
  {
    code: "ME",
    name: "Merseyrail",
    colour: "GColorYellow",
    metro: true,
  },
  {
    code: "NT",
    name: "Northern",
    colour: "GColorDukeBlue",
    metro: true,
  },
  {
    code: "NY",
    name: "North Yorkshire Moors Railway",
    colour: "GColorBlack",
    metro: false,
  }, // maybe an steam train icon as easter egg?
  {
    code: "PC",
    name: "Private Charter",
    colour: "GColorBlack",
    metro: false,
  },
  {
    code: "RT",
    name: "Network Rail",
    colour: "GColorOrange",
    metro: false,
  },
  {
    code: "SE",
    name: "Southeastern",
    colour: "GColorOxfordBlue",
    metro: false,
  },
  {
    code: "SJ",
    name: "Sheffield Supertram",
    colour: "GColorDukeBlue",
    metro: true, // this is a tram?
  },
  {
    code: "SN",
    name: "Southern",
    colour: "GColorMayGreen",
    metro: true,
  },
  {
    code: "SP",
    name: "Swanage Railway",
    colour: "GColorBlack",
  },
  {
    code: "SR",
    name: "ScotRail",
    colour: "GColorDukeBlue",
    metro: false,
  },
  {
    code: "SW",
    name: "South Western Railway",
    colour: "GColorOxfordBlue",
    metro: false,
  },
  {
    code: "TL",
    name: "Thameslink",
    colour: "GColorBrilliantRose",
    metro: true,
  },
  {
    code: "TP",
    name: "TransPennine Express",
    colour: "GColorVividCerulean",
    metro: false,
  },
  {
    code: "TW",
    name: "Tyne and Wear Metro",
    colour: "GColorYellow",
    metro: true,
  },
  {
    code: "VT",
    name: "Avanti West Coast",
    colour: "GColorMidnightGreen",
    metro: false,
  },
  {
    code: "WR",
    name: "West Coast Railway Co",
    colour: "GColorBulgarianRose",
    metro: false,
  },
  {
    code: "XC",
    name: "CrossCountry",
    colour: "GColorJazzberryJam",
    metro: false,
  },
  {
    code: "XR",
    name: "Elizabeth Line",
    colour: "GColorIndigo",
    metro: true,
  },
  {
    code: "ZB",
    name: "Bus Station",
    colour: "GColorBlack",
    metro: true,
  }, // may need bus icon
  {
    code: "ZF",
    name: "Ferry Terminal",
    colour: "GColorCobaltBlue",
    metro: true,
  },
  {
    code: "ZM",
    name: "West Somerset Railway",
    colour: "GColorBlack",
    metro: false,
  },
];

/*
operators.bin format:
name = 32 bytes
colour = 1 byte
type = 1 byte - may expand in future
----------------
stride = 34 bytes

the pebble app uses an index to lookup the operator info
by multiplying the index by 33 to find the start, then reading the next 33 bytes
*/

const file = fs.openSync(new URL("operators.bin", import.meta.url), "w");

for (const operator of operators) {
  const name = operator.name.slice(0, 32).padEnd(32, "\0");
  fs.writeSync(file, name);

  const colour = Buffer.from([pebbleColours[operator.colour]]);
  fs.writeSync(file, colour);

  const type = Buffer.from([operator.metro ? 0b00000001 : 0b00000000]);
  fs.writeSync(file, type);
}

fs.closeSync(file);
