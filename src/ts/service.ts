import { fetchJSON } from "./fetch";
import { TrainServiceDetails } from "./types/service";

export function getServiceInfo(
  serviceID: string,
  callback: (service: TrainServiceDetails) => void
) {
  fetchJSON(
    `https://rail-app-tau.vercel.app/api/service/${serviceID}`,
    (err, response: TrainServiceDetails) => {
      if (err) {
        console.error("Error fetching service info", err);
        return;
      }

      callback(response);
    }
  );
}
