import { Location } from "./location";
import { TrainTime } from "./time";

export interface TrainService {
  origin: Location;
  destination: Location;
  rid: string;
  operator: string;
  operatorCode: string;
  isPassengerService: boolean;
  isCharter: boolean;
  isCancelled: boolean;

  departure: TrainTime;

  platform: string;
}

export interface TrainLocationEntry {
  location: Location;
  platform: string;

  arrival: TrainTime;
  departure: TrainTime;

  lateness?: string;
  skipped: boolean;
}

export interface CurrentLocation {
  location: Location;
  state: "travelling" | "arrived";
}

export interface TrainServiceDetails {
  rid: string;
  operator: string;
  operatorCode: string;
  isCancelled: boolean;
  cancelReason?: string;
  delayReason?: string;
  locations: TrainLocationEntry[];
  currentLocation: CurrentLocation;
}
