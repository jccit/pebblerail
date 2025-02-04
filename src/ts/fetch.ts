export function fetchJSON(
  url: string,
  callback: (error: Error | null, data?: any) => void
): void {
  const req = new XMLHttpRequest();

  req.onload = () => {
    if (req.status >= 200 && req.status < 300) {
      const json = JSON.parse(req.responseText);
      callback(null, json);
    } else {
      callback(new Error(`Request failed with status ${req.status}`));
    }
  };

  req.onerror = () => {
    callback(new Error("Network error"));
  };

  req.open("GET", url, true);
  req.send();
}

export function fetchBinary(
  url: string,
  callback: (arrayBuffer: Uint8Array | null) => void
) {
  const req = new XMLHttpRequest();
  req.responseType = "arraybuffer";

  req.onload = () => {
    const arrayBuffer = req.response;
    if (arrayBuffer) {
      const byteArray = new Uint8Array(arrayBuffer);
      callback(byteArray);
    } else {
      callback(null);
    }
  };

  req.onerror = () => callback(null);
  req.open("GET", url, true);
  req.send();
}

export function postJSON(
  url: string,
  data: Record<string, any>,
  callback: (error: Error | null, data?: any) => void
) {
  const req = new XMLHttpRequest();

  req.onload = () => {
    if (req.status >= 200 && req.status < 300) {
      const json = JSON.parse(req.responseText);
      callback(null, json);
    } else {
      callback(new Error(`Request failed with status ${req.status}`));
    }
  };

  req.onerror = () => {
    callback(new Error("Network error"));
  };

  req.open("POST", url, true);
  req.setRequestHeader("Content-Type", "application/json");
  req.send(JSON.stringify(data));
}
