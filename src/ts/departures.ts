import { fetchJSON } from "./fetch";
import { DepartureBoard, TrainService } from "./types/departureBoard";

export function getDepartureBoard(
  crs: string,
  callback: (departures: TrainService[]) => void
) {
  fetchJSON(
    `https://rail-app-tau.vercel.app/api/departures/${crs}`,
    (err, response: DepartureBoard) => {
      if (err) {
        console.error("Error fetching departures", err);
        return;
      }

      callback(response.services);
    }
  );
}
