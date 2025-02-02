Pebble.addEventListener("ready", function (e) {
  console.log("PKJS ready, sending jsReady message");
  Pebble.sendAppMessage(
    {
      jsReady: 1,
    },
    function () {
      console.log("jsReady message sent");
    },
    function (e) {
      console.log("jsReady message failed: " + e);
    }
  );
});

Pebble.addEventListener("appmessage", function (e) {
  var dict = e.payload;

  console.log("Got message: " + JSON.stringify(dict));
});
