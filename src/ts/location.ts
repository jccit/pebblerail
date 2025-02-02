let lastLocation: GeolocationPosition | null = null;
let locationCallback: (position: GeolocationPosition) => void;

function locationSuccess(position: GeolocationPosition) {
  console.log("location", position);
  lastLocation = position;
  if (locationCallback) {
    locationCallback(position);
  }
}

function locationError(error: GeolocationPositionError) {
  console.log("location error", error);
}

/**
 * Triggers the pebblekit location api
 */
export function requestLocation() {
  console.log("Requesting location");
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
  }
}
