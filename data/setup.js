// Store the current list of WLAN networks
let networks;

// Store the interval for checking the availability of the Alraune home page after the restart
// Is created after the network settings are sent to the server
let interval;

// Wait until the page is loaded, so we can add listenerst to various html elements
document.addEventListener('DOMContentLoaded', function() {

  // Fetch the current wireless networks from the Alraune
  scanNetworks();

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

  // Do the actual setup, if the user clicks on the setup button
  document.getElementById('setup').addEventListener('click', function (event) {
    event.preventDefault();
    doSetup();  
  }, false);

  // Scan the wireless networks again, if the user clicks on the scan button
  document.getElementById('scan').addEventListener('click', function (event) {
    event.preventDefault();
    scanNetworks();
  }, false);

  // Register a listener on the wlan drop down.
  // Depending on the encryption of the selected network the password field is not needed
  document.getElementById('alraune-wlan-ssid').addEventListener('change', function (event) {
    event.preventDefault();
    let selectedNetwork = this.value;
    document.getElementById('alraune-wlan-password').disabled = !networks[selectedNetwork];
  }, false);
}, false);

// Do the actual setup by sending the needed information about name, ssid, password and birthday to the Alraune
// The Alraune will store the given data and send a 202 - ACCEPTED to the client and restart
// This function creates an interval for checking the availability of the Alraune homepage
function doSetup(){
  body = {
    'name': document.getElementById('alraune-name').value,
    'ssid': document.getElementById('alraune-wlan-ssid').value,
    'password': document.getElementById('alraune-wlan-password').disabled ? '' : document.getElementById('alraune-wlan-password').value,
    'birthdayDay': document.getElementById('alraune-birthday').valueAsDate.getDate(),
    'birthdayMonth': document.getElementById('alraune-birthday').valueAsDate.getMonth()
  };
  fetch('/wlan/setup', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'}, 
    body: JSON.stringify(body)
  })
  .then(res => {
    console.log('Request complete! response:', res);
    showLoadingScreen();
    interval = setInterval(function() {
      checkAvailability();
    }, 30000);
  })
  .catch(error => console.error('Error while fetching data: ', error));
}

// Let the Alraune scan the wireless networks in the area and receive the list as JSON
// Parse the JSON list and store the SSID and the security status in the 'networks' variable
// When fill the network dropdown with the names of the networks
function scanNetworks(){
  console.log('Fetch data')
  fetch('/wlan/scan')
  .then(res => res.json())
  .then(data => {
      console.log('Data fetched',data);      
      var select = document.getElementById('alraune-wlan-ssid');
      for (var i=0; i<select.length;  i++){
        select.remove(i);
      }
      networks = {};
      data.forEach(wlan => {
        networks[wlan.ssid] = wlan.encryption;
        var option = document.createElement('option');
        if (wlan.encryption){
          option.text = '🔐 ' + wlan.ssid;
        } else {
          option.text = '🔓 ' + wlan.ssid;
        }
        option.value = wlan.ssid;
        select.add(option);
      });
  })
  .then(() => {
    let selectedNetwork = document.getElementById('alraune-wlan-ssid').value;
    document.getElementById('alraune-wlan-password').disabled = !networks[selectedNetwork];
  })
  .catch(error => console.error('Error while fetching data: ', error));
}

// Check the availability of the Alraune homepage
// If /wlan/status says we are connected to the WLAN after the reboot, the browser redirects to the homepage
function checkAvailability(){
  console.log('Check availability')
  fetch('http://alraune.local/wlan/status')
  .then(res => res.json())
  .then(data => {
    clearInterval(interval);
    if (data.connectedToSsid){
      window.location.href = 'http://alraune.local';
    } else {
      hideLoadingScreen();
      alert('Die Alraune konnte sich nicht mit deinem WLAN verbinden. Bist du sicher, dass du die richtigen Zugangsdaten eingegeben hast?');
    }
  })
  .catch(error => console.log('Setup is not finished yet. Trying again later', error));
}

// Show loading alraune loading screen
function showLoadingScreen(){
  document.getElementById('loading').style.display = "block";
}

// Hide loading alraune loading screen
function hideLoadingScreen(){
  document.getElementById('loading').style.display = "none";
}