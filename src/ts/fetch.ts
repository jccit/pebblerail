export function postJSON(
  url: string,
  data: Record<string, any>,
  callback: (error: Error | null, data?: any) => void
) {
  const req = new XMLHttpRequest();

  console.log(`POSTing to ${url} with data: ${JSON.stringify(data)}`);

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
