import { config } from "./config";
import { postJSON } from "./fetch";

function showError(message: string, error: Error) {
  Pebble.showSimpleNotificationOnPebble(
    "Timeline Error",
    `${message}: ${error.message}`
  );
}

export function pinCallingPoint(
  serviceID: string,
  crs: string,
  type: "departure" | "arrival"
) {
  Pebble.getTimelineToken(
    (token) => {
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
    },
    (error) => {
      console.log(`Error getting timeline token: ${error}`);
      showError("Timeline token error", error);
    }
  );
}
