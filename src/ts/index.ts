import { fetchJSON, fetchBinary } from "./fetch";
import { readStationFile } from "./stationFile";

Pebble.addEventListener("ready", (e) => {
  console.log("PKJS ready, sending jsReady message");
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

    console.log("got array", response.length);

    const file = readStationFile(response);

    console.log("stations", JSON.stringify(file["SOP"]));
  });
}
