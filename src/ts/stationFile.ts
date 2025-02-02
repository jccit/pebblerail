import { Station } from "./types/station";
import { StationFile } from "./gen/stations";

export function readStationFile(buffer: Uint8Array): Record<string, Station> {
  const stationFile = StationFile.decode(buffer);
  return stationFile.stations as Record<string, Station>;
}
