import { config } from "./config";
import { DepartureBoard } from "./types/departureBoard";
import { TrainService } from "./types/service";

export async function getDepartureBoard(crs: string): Promise<TrainService[]> {
  const board: DepartureBoard = await PebbleTS.fetchJSON(
    `${config.service}/api/departures/${crs}`
  );

  return board.services;
}
