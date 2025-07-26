export interface Station {
  name: string;
  crs: string;
  latitude: number;
  longitude: number;
}

export interface StationWithDistance extends Station {
  distance: string;
}

export interface KBStation {
  Name: string;
  CrsCode: string;
  SixteenCharacterName: string;
  Address: {
    "com:PostalAddress": {
      "add:A_5LineAddress": {
        "add:Line": string[];
        "add:Postcode": string;
      };
    };
  };
  Latitude: string;
  Longitude: string;
  StationOperator: string;
  Staffing: {
    StaffingLevel: string;
    ClosedCircuitTelevision: {
      Available: boolean;
    };
  };
  TrainOperatingCompanies: {
    TocRef: string | string[];
  };
}

export interface KBStationResponse {
  StationList: {
    Station: KBStation[];
  };
}
