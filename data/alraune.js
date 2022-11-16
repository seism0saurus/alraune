// Store the volume level of the Alraune
let volume = 0;

// Wait until the page is loaded, so we can add listenerst to various html elements
document.addEventListener('DOMContentLoaded', function() {

  // Request current values of the plant
  fetchData();
  
  // Get new data every minute
  const interval = setInterval(function() {
    fetchData();
  }, 60000);


  // Make the elements collapsible
  var coll = document.getElementsByClassName('collapsible');
  var i;  
  for (i = 0; i < coll.length; i++) {
    if (coll[i].classList.contains('active')) {
      var content = coll[i].nextElementSibling;
      content.style.border = 'solid 4px #6e7491'
      content.style.maxHeight = content.scrollHeight + 'px';
    }
    coll[i].addEventListener('click', function() {
      this.classList.toggle('active');
      var content = this.nextElementSibling;
      if (content.style.maxHeight){
        content.style.maxHeight = null;
      } else {
        content.style.maxHeight = content.scrollHeight + 'px';
      }
      if (content.style.border){
        content.style.border = null;
      } else {
        content.style.border = 'solid 4px #6e7491'
      }
    });
  }

  // Play a sound, if the Alraune is tickled
  var audio = new Audio('tickle.mp3');
  var alraune = document.getElementById('alraune-img');
  alraune.addEventListener('mouseover', function() {
    audio.play();
  });

  // Tell the system to mute
  document.getElementById('mute').addEventListener('click', function (event) {
    event.preventDefault();
    volume = 0;
    body = { 'volume': volume };
    postJson("/volume", body);  
  }, false);
  
  // Tell the system to reduce the volume
  document.getElementById('quieter').addEventListener('click', function (event) {
    event.preventDefault();
    if (volume > 3){
      volume -= 3;
    } else {
      volume = 0;
    }
    body = { 'volume': volume };
    postJson("/volume", body);  
  }, false);
 
  // Tell the system to increase the volume
  document.getElementById('louder').addEventListener('click', function (event) {
    event.preventDefault();
    if (volume < 28){
      volume += 3;
    } else {
      volume = 30;
    }
    body = { 'volume': volume };
    postJson("/volume", body);  
  }, false);
  
  // Tell the system to play a random water refill test sound
  document.getElementById('water').addEventListener('click', function (event) {
    event.preventDefault();
    body = { 'sound': 'water' };
    postJson("/play", body);  
  }, false);
  
  // Tell the system to play a random birthday test sound
  document.getElementById('birthday').addEventListener('click', function (event) {
    event.preventDefault();
    body = { 'sound': 'birthday' };
    postJson("/play", body);  
  }, false);
  
  // Tell the system to play a random christmas test sound
  document.getElementById('christmas').addEventListener('click', function (event) {
    event.preventDefault();
    body = { 'sound': 'christmas' };
    postJson("/play", body);  
  }, false);

  // Start the recallibration of the moisture sensor
  document.getElementById('recalibrate').addEventListener('click', function (event) {
    event.preventDefault();
    postJson("/recalibrate", {});  
  }, false);
}, false);


// Post a given javascript object to the given path
function postJson(path, body){
  fetch(path, {
    method: "POST",
    headers: {'Content-Type': 'application/json'}, 
    body: JSON.stringify(body)
  })
  .then(res => {
    console.log("Request complete! response:", res);
    document.getElementById('loudness').value = volume;
  })
  .catch(error => console.error('Error while fetching data: ', error))
  ;
}

// Fetch the data object from the Alraune and set the elements of the page to the correct values
function fetchData(){
  fetch('/json')
  .then(res => res.json())
  .then(data => {
      console.log('Data fetched',data);
      document.title = data.title;
      document.getElementById('moisture').innerHTML = data.moisture;
      document.getElementById('treshold').innerHTML = data.sensorThreshold;
      document.getElementById('memory').innerHTML = data.memory;
      document.getElementById('needsWater').innerHTML = data.needsWater;
      document.getElementById('needsRefill').innerHTML = data.needsRefill;
      volume = data.volume;
      document.getElementById('loudness').value = volume;
  })
  .catch(error => console.error('Error while fetching data: ', error));
}