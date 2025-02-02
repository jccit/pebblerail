import type { Location } from "./location";

/**
 * Raw response for a departure board from Darwin
 */
export interface DarwinDepartureBoard {
  GetDepartureBoardResponse: {
    GetStationBoardResult: {
      generatedAt: string;
      locationName: string;
      crs: string;
      platformAvailable: boolean;
      trainServices: {
        service: TrainService[];
      };
    };
  };
}

/**
 * Cleaned departure board
 */
export interface DepartureBoard {
  generatedAt: Date;
  locationName: string;
  crs: string;
  services: TrainService[];
}

export interface TrainService {
  std: string;
  etd: string;
  platform: string;
  operator: string;
  operatorCode: string;
  serviceID: string;
  serviceType: string;
  length: string;
  destination: {
    location: Location;
  };
}
