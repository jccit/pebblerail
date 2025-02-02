import { TrainService } from "./types/departureBoard";
import { StationWithDistance } from "./types/station";

function sendItem(items: Record<string, any>, index: number) {
  const item = items[index];

  Pebble.sendAppMessage(
    item,
    () => {
      console.log("sent item", JSON.stringify(item));
      index++;

      if (index < items.length) {
        sendItem(items, index);
      } else {
        console.log("sent all items");
      }
    },
    () => {
      console.log("failed to send item", index);
    }
  );
}

export function sendStationList(stations: StationWithDistance[]) {
  const serialised = stations.slice(0, 5).map((station) => ({
    objectType: "stationList",
    locationName: station.name,
    crs: station.crs,
    distance: station.distance,
  }));

  sendItem(serialised, 0);
}

export function sendDepartureList(departures: TrainService[]) {
  const departuresToSend = departures.slice(0, 10);

  const serialised = departuresToSend.map((departure) => {
    let departureTime = departure.std;

    if (departure.etd !== "On time") {
      departureTime = departure.etd;
    }

    return {
      objectType: "departureList",
      count: departuresToSend.length,
      serviceID: departure.serviceID,
      locationName: departure.destination.location.locationName,
      time: departureTime,
      platform: departure.platform || "-1",
    };
  });

  console.log(`Sending ${serialised.length} departures`);

  sendItem(serialised, 0);
}
