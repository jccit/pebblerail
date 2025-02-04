import { config } from "./config";
import { fetchJSON } from "./fetch";
import { DepartureBoard } from "./types/departureBoard";
import { TrainService } from "./types/service";

export function getDepartureBoard(
  crs: string,
  callback: (departures: TrainService[]) => void
) {
  fetchJSON(
    `${config.service}/api/departures/${crs}`,
    (err, response: DepartureBoard) => {
      if (err) {
        console.error("Error fetching departures", err);
        return;
      }

      callback(response.services);
    }
  );
}
