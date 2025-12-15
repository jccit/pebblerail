import { config } from "./config";
import { postJSON } from "./fetch";
import { getServiceInfo } from "./service";

function showError(message: string, error: Error) {
  Pebble.showSimpleNotificationOnPebble(
    "Timeline Error",
    `${message}: ${error.message}`
  );
}

async function postTimeline(
  serviceID: string,
  crs: string,
  type: "departure" | "arrival"
) {
  try {
    const token = await PebbleTS.getTimelineToken();

    console.log(`Timeline token: ${token}`);
    postJSON(
      `${config.service}/api/timeline`,
      {
        userToken: token,
        serviceID,
        crs,
        type,
      },
      (error: Error | null, data?: any) => {
        if (error) {
          console.log(`Error pinning calling point: ${error}`);
          showError("Couldn't pin to timeline", error);
        } else {
          console.log(`Calling point pinned: ${data}`);
          Pebble.showSimpleNotificationOnPebble(
            `${type === "departure" ? "Departure" : "Arrival"} pinned!`,
            `It may take a few minutes to sync. You'll be notified when it's ready.`
          );
        }
      }
    );
  } catch (error) {
    console.log(`Error getting timeline token: ${error}`);
    showError(
      "Timeline token error",
      new Error("Failed to get timeline token")
    );
  }
}

export async function pinCallingPoint(
  serviceID: string,
  crs: string,
  type: "departure" | "arrival"
) {
  try {
    const service = await getServiceInfo(serviceID);

    const callingPoint = service.locations.find(
      (location) => location.location.crs === crs
    );

    if (!callingPoint) {
      throw new Error(`Calling point not found: ${crs}`);
    }

    const time = new Date(
      callingPoint.arrival.scheduled ||
        callingPoint.departure.scheduled ||
        callingPoint.departure.actual ||
        new Date()
    );

    console.log(callingPoint);

    PebbleTS.insertTimelinePin({
      id: `${serviceID}-${crs}-${type}`,
      time,
      layout: {
        type: "genericPin",
        title: `${type === "departure" ? "Departing from" : "Arriving at"} ${
          callingPoint.location.locationName
        }`,
        tinyIcon: "app://images/TRAIN",
      },
      reminders: [
        {
          time: new Date(time.getTime() - 10 * 60000),
          layout: {
            type: "genericReminder",
            title: `Train ${
              type === "departure" ? "departing from" : "arriving at"
            } ${callingPoint.location.locationName}`,
            tinyIcon: "app://images/TRAIN",
          },
        },
      ],
      actions: [
        {
          type: "openWatchApp",
          title: "Open app",
          launchCode: serviceID,
        },
      ],
    });

    Pebble.showSimpleNotificationOnPebble(
      `${type === "departure" ? "Departure" : "Arrival"} pinned!`,
      `Timeline will not show delays.`
    );
    return;
  } catch (error) {
    console.log(`Local timeline pin failed: ${error}`);
  }

  console.log("Attempting server timeline pin");
  postTimeline(serviceID, crs, type);
}
