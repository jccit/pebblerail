let lastLocation: GeolocationPosition | null = null;
let locationCallback: (position: GeolocationPosition) => void;
let requestingLocation = false;

const fakeLocation: Record<string, { latitude: number, longitude: number }> = {
  liverpool: {
    latitude: 53.4066873,
    longitude: -2.9818569,
  },
  londonNorth: {
    latitude: 51.5293076,
    longitude: -0.1322747
  }
}

function getFakeLocation(location: keyof typeof fakeLocation): GeolocationPosition {
    const coords = fakeLocation[location] as GeolocationCoordinates;

    return {
      coords
  } as GeolocationPosition
}

function isEmulator() {
  return !navigator.userAgent;
}

function locationSuccess(position: GeolocationPosition) {
  console.log("location", position);
  lastLocation = position;
  requestingLocation = false;
  if (locationCallback) {
    locationCallback(position);
  }
}

function locationError(error: GeolocationPositionError) {
  console.log("location error", error);
  requestingLocation = false;
}

/**
 * Triggers the pebblekit location api
 */
export function requestLocation() {
  if (isEmulator()) {
    console.log("Emulator detected, skipping location request");
    
    lastLocation = getFakeLocation("liveerpool");
    
    return;
  }

  console.log("Requesting location");
  requestingLocation = true;
  console.log("userAgent", navigator.userAgent);
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError);
}

/**
 * Returns the current location. Make sure to call requestLocation() first.
 * If we have it already, the callback is triggered immediately.
 */
export function getLocation(callback: (position: GeolocationPosition) => void) {  
  if (lastLocation) {
    callback(lastLocation);
  } else {
    locationCallback = callback;

    if (!requestingLocation) {
      requestLocation();
    }
  }
}
