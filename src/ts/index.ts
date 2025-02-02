import { fetchJSON, fetchBinary } from "./fetch";
import {
  countStations,
  findClosestStations,
  readStationFile,
} from "./stationFile";
import { requestLocation, getLocation } from "./location";
import { sendStationList } from "./data";

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

  var command = dict.dataRequest;

  switch (command) {
    case "stationList":
      console.log("Station list requested");
      getStationList();
      break;
    default:
      console.log("Unknown command: " + command);
      break;
  }
});

async function getStationList() {
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
