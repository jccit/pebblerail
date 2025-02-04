import { TrainService } from "./service";

export interface DepartureBoard {
  services: TrainService[];
  locationName: string;
  crs: string;
  stationManager: string;
  stationManagerCode: string;
}
