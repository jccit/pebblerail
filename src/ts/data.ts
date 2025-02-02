import { StationWithDistance } from "./types/station";

function sendItem(key: string, items: Array<string>, index: number) {
  const dict: Record<string, string> = {};
  dict[key] = items[index];

  Pebble.sendAppMessage(
    dict,
    () => {
      console.log("sent item", key, index);
      index++;

      if (index < items.length) {
        sendItem(key, items, index);
      } else {
        console.log("sent all items");
      }
    },
    () => {
      console.log("failed to send item", key, index);
    }
  );
}

export function sendStationList(stations: StationWithDistance[]) {
  const serialised = stations
    .slice(0, 5)
    .map((station) => `${station.name};${station.distance}`);

  sendItem("stationList", serialised, 0);
}
