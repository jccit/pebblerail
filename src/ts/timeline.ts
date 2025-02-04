import { config } from "./config";
import { postJSON } from "./fetch";

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
          } else {
            console.log(`Calling point pinned: ${data}`);
          }
        }
      );
    },
    (error) => {
      console.log(`Error getting timeline token: ${error}`);
    }
  );
}
