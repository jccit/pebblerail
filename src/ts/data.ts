import { calculateTime } from "./time";
import { TrainService, TrainServiceDetails } from "./types/service";
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
    const time = calculateTime(departure.departure);
    let timeString = time?.scheduled || "00:00";

    if (time?.isLate) {
      timeString = `${time.actual} (${time.lateness}m late)`;
    }

    return {
      objectType: "departureList",
      count: departuresToSend.length,
      serviceID: departure.rid,
      locationName: departure.destination.locationName,
      time: timeString,
      platform: departure.platform || "-1",
    };
  });

  console.log(`Sending ${serialised.length} departures`);

  if (serialised.length === 0) {
    console.log("Sending empty departure list");
    sendItem([{ objectType: "departureList", count: 0 }], 0);
    return;
  }

  sendItem(serialised, 0);
}

export function sendServiceInfo(service: TrainServiceDetails) {
  const callingPoints = service.locations;

  const serialised = callingPoints.map((location) => {
    const time = calculateTime(
      location.arrival.scheduled ? location.arrival : location.departure
    );
    let timeString = time?.scheduled || "00:00";

    if (time?.isLate) {
      timeString = `${time.actual} (${time.lateness}m late)`;
    }

    return {
      objectType: "serviceInfo",
      count: callingPoints.length,
      locationName: location.location.locationName,
      crs: location.location.crs,
      time: timeString,
      platform: location.platform || "-1",
    };
  });

  console.log(`Sending ${serialised.length} calling points`);

  if (serialised.length === 0) {
    console.log("Sending empty calling points list");
    sendItem([{ objectType: "serviceInfo", count: 0 }], 0);
    return;
  }

  sendItem(serialised, 0);
}
