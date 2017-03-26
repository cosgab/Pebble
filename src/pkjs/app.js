// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

// 8fbb9ed659225947db3954e6f9ef7a8e
var myAPIKey;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      console.log(responseText);
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      // console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].description;      
      // console.log("Conditions are " + conditions);

      var sunrise = json.sys.sunrise;      
      // console.log("Sunrise at " + sunrise);
      var sunset = json.sys.sunset;      
      // console.log("Sunset at " + sunset);      
      
      var place = json.name;      
      // console.log("Place at " + place);
      // Assemble dictionary using our keys
      var dictionary = {
        "type" : "sun",
        "temperature": temperature,
        "conditions": conditions,
        "sunrise": sunrise,
        "sunset": sunset,
        "place": place
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    // getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    var dict = e.payload;
    console.log('Got message: ' + JSON.stringify(dict));
    if(dict['API_key']) {
      // The RequestData key is present, read the value
      myAPIKey = dict['API_key'];
      console.log("AppMessage received! " + myAPIKey );
    }
    getWeather();
  }                     
);
