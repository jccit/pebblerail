import { config } from "./config";
import { DepartureBoard } from "./types/departureBoard";
import { TrainService } from "./types/service";

export async function getDepartureBoard(crs: string): Promise<TrainService[]> {
  const response = await fetch(`${config.service}/api/departures/${crs}`);
  const board: DepartureBoard = await response.json();

  if (!response.ok) {
    throw new Error(`Failed to fetch departure board: ${response.statusText}`);
  }

  return board.services;
}
