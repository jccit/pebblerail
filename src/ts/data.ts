import { calculateTime } from "./time";
import {
  CallingPointState,
  TrainLocationEntry,
  TrainService,
  TrainServiceDetails,
} from "./types/service";
import { StationWithDistance } from "./types/station";

function sendSingleMessage(item: Record<string, any>, callback: () => void) {
  Pebble.sendAppMessage(item, callback, () => {
    console.log("failed to send item", JSON.stringify(item));
  });
}

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
      operatorCode: departure.operatorCode,
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

function getDestination(callingPoints: TrainLocationEntry[]) {
  for (let i = callingPoints.length - 1; i >= 0; i--) {
    if (!callingPoints[i].skipped) {
      return callingPoints[i].location.crs;
    }
  }
  return "";
}

function getLocationIndex(callingPoints: TrainLocationEntry[], crs: string) {
  for (let i = 0; i < callingPoints.length; i++) {
    if (callingPoints[i].location.crs === crs) {
      return i;
    }
  }
  return -1;
}

export function sendServiceInfo(service: TrainServiceDetails, fromCrs: string) {
  const originIndex = getLocationIndex(service.locations, fromCrs);
  const callingPoints = service.locations.slice(
    originIndex < 0 ? 0 : originIndex
  );

  const serialised = callingPoints.map((location) => {
    const time = calculateTime(
      location.arrival.scheduled ? location.arrival : location.departure
    );
    let timeString = time?.scheduled || "00:00";

    if (time?.isLate) {
      timeString = `${time.actual} (${time.lateness}m late)`;
    }

    let state = CallingPointState.NotArrived;

    if (location.arrival.actual) {
      state = CallingPointState.Arrived;
    }

    if (location.departure.actual) {
      state = CallingPointState.Departed;
    }

    if (location.skipped) {
      state = CallingPointState.Skipped;
    }

    return {
      objectType: "callingPoint",
      count: callingPoints.length,
      locationName: location.location.locationName,
      crs: location.location.crs,
      time: timeString,
      platform: location.platform || "-1",
      callingPointState: state,
    };
  });

  const destination = getDestination(callingPoints);

  const serviceInfo = {
    objectType: "serviceInfo",
    serviceID: service.rid,
    origin: service.locations[0].location.crs,
    destination,
    operatorCode: service.operatorCode,
    isCancelled: service.isCancelled,
    cancelReason: service.cancelReason || "",
    delayReason: service.delayReason || "",
  };

  console.log("Sending service info", JSON.stringify(serviceInfo));
  sendSingleMessage(serviceInfo, () => {
    console.log(`Sending ${serialised.length} calling points`);

    if (serialised.length === 0) {
      console.log("Sending empty calling points list");
      sendItem([{ objectType: "serviceInfo", count: 0 }], 0);
      return;
    }

    sendItem(serialised, 0);
  });
}
