interface PebbleKitJS {
  addEventListener<T extends keyof PebbleEvents>(
    event: T,
    callback: (e: PebbleEvents[T]) => void
  ): void;

  sendAppMessage(
    message: Record<string, any>,
    successCallback: (e: any) => void,
    errorCallback: (e: any) => void
  ): void;

  getTimelineToken(
    successCallback: (e: any) => void,
    errorCallback: (e: any) => void
  ): void;

  showSimpleNotificationOnPebble(title: string, body: string): void;
}

interface AppMessageEvent {
  payload: Record<string, any>;
}

interface PebbleEvents {
  ready: any;
  appmessage: AppMessageEvent;
}

declare global {
  var Pebble: PebbleKitJS;
}

export {};
