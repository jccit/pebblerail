import { fetchJSON, fetchBinary } from "./fetch";
import {
  countStations,
  findClosestStations,
  readStationFile,
} from "./stationFile";
import { requestLocation, getLocation } from "./location";
import { sendDepartureList, sendStationList } from "./data";
import { getDepartureBoard } from "./departures";

Pebble.addEventListener("ready", (e) => {
  console.log("PKJS ready, sending jsReady message");
  requestLocation();
  Pebble.sendAppMessage(
    {
      jsReady: 1,
    },
    () => {
      console.log("jsReady message received");
    },
    (e) => {
      console.log("jsReady message failed: " + JSON.stringify(e));
    }
  );
});

Pebble.addEventListener("appmessage", (e) => {
  var dict = e.payload;

  console.log("Got message: " + JSON.stringify(dict));

  var command = dict.request;

  switch (command) {
    case "stationList":
      console.log("Station list requested");
      getStationList();
      break;
    case "departures":
      const crs = dict.requestData;
      console.log(`Departures for ${crs} requested`);
      getDepartures(crs);
      break;
    default:
      console.log("Unknown command: " + command);
      break;
  }
});

function getStationList() {
  console.log("starting fetch");

  fetchBinary("https://rail-app-tau.vercel.app/api/stations", (response) => {
    if (!response) {
      console.log("no response");
      return;
    }

    console.log("got binary of length", response.length);
    console.log("decoding station file");

    readStationFile(response);

    console.log(`decoded ${countStations()} stations`);

    getLocation((position) => {
      const closest = findClosestStations(
        position.coords.latitude,
        position.coords.longitude,
        5
      );

      console.log("closest stations", JSON.stringify(closest));

      sendStationList(closest);
    });
  });
}

function getDepartures(crs: string) {
  getDepartureBoard(crs, (departures) => {
    sendDepartureList(departures);
  });
}
