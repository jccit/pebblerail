import { config } from "./config";
import { fetchJSON } from "./fetch";
import { TrainServiceDetails } from "./types/service";

export function getServiceInfo(
  serviceID: string,
  callback: (service: TrainServiceDetails) => void
) {
  fetchJSON(
    `${config.service}/api/service/${serviceID}`,
    (err, response: TrainServiceDetails) => {
      if (err) {
        console.error("Error fetching service info", err);
        return;
      }

      callback(response);
    }
  );
}
