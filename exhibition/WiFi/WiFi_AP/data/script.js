function buttonClick(msg) {

  fetch("/pressed?button=" + msg)  //go to directory /pressed; send parameter button=msg
    .then(response => response.text())
    .then(text => {
      console.log("ESP32 replied: " + text);
    })
    .catch(err => {
      console.log("!! " + err);
    });
}