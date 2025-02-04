import { Station, StationWithDistance } from "./types/station";
import { StationFile } from "./gen/stations";
import "core-js/modules/es.object.values";

let stationFile: Record<string, Station> | null = null;

export function readStationFile(buffer: Uint8Array) {
  const decoded = StationFile.decode(buffer);
  stationFile = decoded.stations as Record<string, Station>;
}

export function loadStationFileJSON(json: string) {
  stationFile = JSON.parse(json);
}

export function exportStationFileJSON() {
  return JSON.stringify(stationFile);
}

export function getStation(id: string) {
  return stationFile?.[id];
}

export function countStations() {
  if (!stationFile) {
    return 0;
  }

  return Object.keys(stationFile).length;
}

export function findClosestStations(
  lat: number,
  lon: number,
  count: number
): StationWithDistance[] {
  if (!stationFile) {
    return [];
  }

  const toRad = Math.PI / 180;
  const earthRadiusMiles = 3958.8;

  const stationsWithDistance = Object.values(stationFile).map((station) => {
    const dLat = (station.latitude - lat) * toRad;
    const dLon = (station.longitude - lon) * toRad;
    const lat1 = lat * toRad;
    const lat2 = station.latitude * toRad;
    const a =
      Math.sin(dLat / 2) ** 2 +
      Math.cos(lat1) * Math.cos(lat2) * Math.sin(dLon / 2) ** 2;
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    const distance = earthRadiusMiles * c;
    const distanceStr = distance.toFixed(2) + "mi";
    return { ...station, distance: distanceStr };
  });

  // Sort stations by their numeric distance (parsed from the distance string).
  stationsWithDistance.sort(
    (a, b) => parseFloat(a.distance) - parseFloat(b.distance)
  );

  return stationsWithDistance.slice(0, count);
}
