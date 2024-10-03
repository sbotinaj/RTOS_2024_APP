/**
 * Add gobals here
 */
var seconds 	= null;
var otaTimerVar =  null;
var wifiConnectInterval = null;

/**
 * Initialize functions here.
 */
$(document).ready(function(){
	//getUpdateStatus();
	startDHTSensorInterval();
	$("#connect_wifi").on("click", function(){
		checkCredentials();
	}); 

});   

/**
 * Gets file name and size for display on the web page.
 */        
function getFileInfo() 
{
    var x = document.getElementById("selected_file");
    var file = x.files[0];

    document.getElementById("file_info").innerHTML = "<h4>File: " + file.name + "<br>" + "Size: " + file.size + " bytes</h4>";
}

/**
 * Handles the firmware update.
 */
function updateFirmware() 
{
    // Form Data
    var formData = new FormData();
    var fileSelect = document.getElementById("selected_file");
    
    if (fileSelect.files && fileSelect.files.length == 1) 
	{
        var file = fileSelect.files[0];
        formData.set("file", file, file.name);
        document.getElementById("ota_update_status").innerHTML = "Uploading " + file.name + ", Firmware Update in Progress...";

        // Http Request
        var request = new XMLHttpRequest();

        request.upload.addEventListener("progress", updateProgress);
        request.open('POST', "/OTAupdate");
        request.responseType = "blob";
        request.send(formData);
    } 
	else 
	{
        window.alert('Select A File First')
    }
}

/**
 * Progress on transfers from the server to the client (downloads).
 */
function updateProgress(oEvent) 
{
    if (oEvent.lengthComputable) 
	{
        getUpdateStatus();
    } 
	else 
	{
        window.alert('total size is unknown')
    }
}

/**
 * Posts the firmware udpate status.
 */
function getUpdateStatus() 
{
    var xhr = new XMLHttpRequest();
    var requestURL = "/OTAstatus";
    xhr.open('POST', requestURL, false);
    xhr.send('ota_update_status');

    if (xhr.readyState == 4 && xhr.status == 200) 
	{		
        var response = JSON.parse(xhr.responseText);
						
	 	document.getElementById("latest_firmware").innerHTML = response.compile_date + " - " + response.compile_time

		// If flashing was complete it will return a 1, else -1
		// A return of 0 is just for information on the Latest Firmware request
        if (response.ota_update_status == 1) 
		{
    		// Set the countdown timer time
            seconds = 10;
            // Start the countdown timer
            otaRebootTimer();
        } 
        else if (response.ota_update_status == -1)
		{
            document.getElementById("ota_update_status").innerHTML = "!!! Upload Error !!!";
        }
    }
}

/**
 * Displays the reboot countdown.
 */
function otaRebootTimer() 
{	
    document.getElementById("ota_update_status").innerHTML = "OTA Firmware Update Complete. This page will close shortly, Rebooting in: " + seconds;

    if (--seconds == 0) 
	{
        clearTimeout(otaTimerVar);
        window.location.reload();
    } 
	else 
	{
        otaTimerVar = setTimeout(otaRebootTimer, 1000);
    }
}

/**
 * Gets DHT22 sensor temperature and humidity values for display on the web page.
 */


function getregValues()
{
	$.getJSON('/read_regs.json', function(data) {
		$("#reg_1").text(data["reg1"]);
		$("#reg_2").text(data["reg2"]);
		$("#reg_3").text(data["reg3"]);
		$("#reg_4").text(data["reg4"]);
		$("#reg_5").text(data["reg5"]);
		$("#reg_6").text(data["reg6"]);
		$("#reg_7").text(data["reg7"]);
		$("#reg_8").text(data["reg8"]);
		$("#reg_9").text(data["reg9"]);
		$("#reg_10").text(data["reg10"]);
	});
}

function getDHTSensorValues()
{
	$.getJSON('/dhtSensor.json', function(data) {
		$("#temperature_reading").text(data["temp"]);
		$("#humidity_reading").text(data["humidity"]);
		
	});
}

function getDutyCycle() {

	
    $.getJSON('/show_duty', function(data) {
        $("#duty_cycle_reading").text(data["duty"]);
    });

	
}

function getDutyCycleBuzzer() {

	
    $.getJSON('/show_duty_buzzer', function(data) {
        $("#duty_cycle_buzzer").text(data["duty_buzzer"]);
    });

	
}

function getDutyCycleFan() {

	
    $.getJSON('/show_duty_fan', function(data) {
        $("#duty_cycle_fan").text(data["duty_fan"]);
    });

	
}


/**
 * Sets the interval for getting the updated DHT22 sensor values.
 */

function startDHTSensorInterval()
{
	setInterval(getDHTSensorValues, 1000);    
	setInterval(getDutyCycle,100)
	setInterval(getDutyCycleBuzzer,1000)
	setInterval(getDutyCycleFan,1000)
}

   



/**
 * Clears the connection status interval.
 */
function stopWifiConnectStatusInterval()
{
	if (wifiConnectInterval != null)
	{
		clearInterval(wifiConnectInterval);
		wifiConnectInterval = null;
	}
}

/**
 * Gets the WiFi connection status.
 */
function getWifiConnectStatus()
{
	var xhr = new XMLHttpRequest();
	var requestURL = "/wifiConnectStatus";
	xhr.open('POST', requestURL, false);
	xhr.send('wifi_connect_status');
	
	if (xhr.readyState == 4 && xhr.status == 200)
	{
		var response = JSON.parse(xhr.responseText);
		
		document.getElementById("wifi_connect_status").innerHTML = "Connecting...";
		
		if (response.wifi_connect_status == 2)
		{
			document.getElementById("wifi_connect_status").innerHTML = "<h4 class='rd'>Failed to Connect. Please check your AP credentials and compatibility</h4>";
			stopWifiConnectStatusInterval();
		}
		else if (response.wifi_connect_status == 3)
		{
			document.getElementById("wifi_connect_status").innerHTML = "<h4 class='gr'>Connection Success!</h4>";
			stopWifiConnectStatusInterval();
		}
	}
}

/**
 * Starts the interval for checking the connection status.
 */
function startWifiConnectStatusInterval()
{
	wifiConnectInterval = setInterval(getWifiConnectStatus, 2800);
}

/**
 * Connect WiFi function called using the SSID and password entered into the text fields.
 */
function connectWifi()
{
	// Get the SSID and password
	/*selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val();
	
	$.ajax({
		url: '/wifiConnect.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		headers: {'my-connect-ssid': selectedSSID, 'my-connect-pwd': pwd},
		data: {'timestamp': Date.now()}
	});
	*/
	selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val();
	
	// Create an object to hold the data to be sent in the request body
	var requestData = {
	  'selectedSSID': selectedSSID,
	  'pwd': pwd,
	  'timestamp': Date.now()
	};
	
	// Serialize the data object to JSON
	var requestDataJSON = JSON.stringify(requestData);
	
	$.ajax({
	  url: '/wifiConnect.json',
	  dataType: 'json',
	  method: 'POST',
	  cache: false,
	  data: requestDataJSON, // Send the JSON data in the request body
	  contentType: 'application/json', // Set the content type to JSON
	  success: function(response) {
		// Handle the success response from the server
		console.log(response);
	  },
	  error: function(xhr, status, error) {
		// Handle errors
		console.error(xhr.responseText);
	  }
	});


	//startWifiConnectStatusInterval();
}

/**
 * Checks credentials on connect_wifi button click.
 */
function checkCredentials()
{
	errorList = "";
	credsOk = true;
	
	selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val();
	
	if (selectedSSID == "")
	{
		errorList += "<h4 class='rd'>SSID cannot be empty!</h4>";
		credsOk = false;
	}
	if (pwd == "")
	{
		errorList += "<h4 class='rd'>Password cannot be empty!</h4>";
		credsOk = false;
	}
	
	if (credsOk == false)
	{
		$("#wifi_connect_credentials_errors").html(errorList);
	}
	else
	{
		$("#wifi_connect_credentials_errors").html("");
		connectWifi();    
	}
}

/**
 * Shows the WiFi password if the box is checked.
 */
function showPassword()
{
	var x = document.getElementById("connect_pass");
	if (x.type === "password")
	{
		x.type = "text";
	}
	else
	{
		x.type = "password";
	}
}


function send_register()
{
    // Assuming you have selectedNumber, hours, minutes variables populated from your form
    selectedNumber = $("#selectNumber").val();
    hours = $("#hours").val();
    minutes = $("#minutes").val();
    
    // Create an array for selected days
    var selectedDays = [];
    if ($("#day_mon").prop("checked")) selectedDays.push("1");
	else selectedDays.push("0");
    if ($("#day_tue").prop("checked")) selectedDays.push("1");
	else selectedDays.push("0");
    if ($("#day_wed").prop("checked")) selectedDays.push("1");
	else selectedDays.push("0");
    if ($("#day_thu").prop("checked")) selectedDays.push("1");
	else selectedDays.push("0");
    if ($("#day_fri").prop("checked")) selectedDays.push("1");
	else selectedDays.push("0");
    if ($("#day_sat").prop("checked")) selectedDays.push("1");
	else selectedDays.push("0");
    if ($("#day_sun").prop("checked")) selectedDays.push("1");
	else selectedDays.push("0");

    // Create an object to hold the data to be sent in the request body
    var requestData = {
        'selectedNumber': selectedNumber,
        'hours': hours,
        'minutes': minutes,
        'selectedDays': selectedDays,
        'timestamp': Date.now()
    };

    // Serialize the data object to JSON
    var requestDataJSON = JSON.stringify(requestData);

	$.ajax({
		url: '/regchange.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		data: requestDataJSON, // Send the JSON data in the request body
		contentType: 'application/json', // Set the content type to JSON
		success: function(response) {
		  // Handle the success response from the server
		  console.log(response);
		},
		error: function(xhr, status, error) {
		  // Handle errors
		  console.error(xhr.responseText);
		}
	  });

    // Print the resulting JSON to the console (for testing)
    //console.log(requestDataJSON);
}

/**
 * toogle led function.
 */
function read_reg()
{

	
	$.ajax({
		url: '/readreg.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		//headers: {'my-connect-ssid': selectedSSID, 'my-connect-pwd': pwd},
		//data: {'timestamp': Date.now()}
	});
//	var xhr = new XMLHttpRequest();
//	xhr.open("POST", "/toogle_led.json");
//	xhr.setRequestHeader("Content-Type", "application/json");
//	xhr.send(JSON.stringify({data: "mi información"}));
}


function erase_register()
{
    // Assuming you have selectedNumber, hours, minutes variables populated from your form
    selectedNumber = $("#selectNumber").val();



    // Create an object to hold the data to be sent in the request body
    var requestData = {
        'selectedNumber': selectedNumber,
        'timestamp': Date.now()
    };

    // Serialize the data object to JSON
    var requestDataJSON = JSON.stringify(requestData);

	$.ajax({
		url: '/regchange.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		data: requestDataJSON, // Send the JSON data in the request body
		contentType: 'application/json', // Set the content type to JSON
		success: function(response) {
		  // Handle the success response from the server
		  console.log(response);
		},
		error: function(xhr, status, error) {
		  // Handle errors
		  console.error(xhr.responseText);
		}
	  });

    // Print the resulting JSON to the console (for testing)
    //console.log(requestDataJSON);
}

function toogle_led() 
{	
	$.ajax({
		url: '/toogle_led.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

}


function increaseBrightness() {
    // Obtén el color seleccionado
    let selectedColor = getSelectedColor();
    
    if (selectedColor) {
        let url;

        // Determina la URL en función del color seleccionado
        switch (selectedColor) {
            case 'red':
                url = '/ledR_up';
                break;
            case 'green':
                url = '/ledG_up';
                break;
            case 'blue':
                url = '/ledB_up';
                break;
            default:
                console.error('Invalid color selected');
                return;
        }

        // Envía la solicitud AJAX con la URL correspondiente
        $.ajax({
            url: url,
            dataType: 'json',
            method: 'POST',
            cache: false,
            /* success: function(response) {
                console.log('Brightness increased for color: ' + selectedColor);
            },
            error: function(error) {
                console.error('Error increasing brightness', error);
            } */
        });
    } else {
        console.error('No color selected');
    }
}

function decreaseBrightness() {
    // Obtén el color seleccionado
    let selectedColor = getSelectedColor();
    
    if (selectedColor) {
        let url;

        // Determina la URL en función del color seleccionado
        switch (selectedColor) {
            case 'red':
                url = '/ledR_down';
                break;
            case 'green':
                url = '/ledG_down';
                break;
            case 'blue':
                url = '/ledB_down';
                break;
            default:
                console.error('Invalid color selected');
                return;
        }

        // Envía la solicitud AJAX con la URL correspondiente
        $.ajax({
            url: url,
            dataType: 'json',
            method: 'POST',
            cache: false,
            /* success: function(response) {
                console.log('Brightness decreased for color: ' + selectedColor);
            },
            error: function(error) {
                console.error('Error decreasing brightness', error);
            } */
        });
    } else {
        console.error('No color selected');
    }
}

function getSelectedColor() {
    if (document.getElementById('redRadio').checked) {
        return 'red';
    } else if (document.getElementById('greenRadio').checked) {
        return 'green';
    } else if (document.getElementById('blueRadio').checked) {
        return 'blue';
    } else {
        return null; // No hay color seleccionado
    }
}


/* 
function ledR() 
{	
	$.ajax({
		url: '/ledR.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

}

function ledG() 
{	
	$.ajax({
		url: '/ledG.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

}

function ledB() 
{	
	$.ajax({
		url: '/ledB.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

}

function increaseBrightness() 
{	
	$.ajax({
		url: '/pwmUP.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

}

function decreaseBrightness() 
{	
	$.ajax({
		url: '/pwmDOWN.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

} */

function brigthness_up() 
{	
	$.ajax({
		url: '/toogle_led.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

}




//Control led rgb

function sendConfigRGB()
{
	minTemperatureR = $("#ledR_min_temp").val();
	maxTemperatureR = $("#ledR_max_temp").val();
	intensityR = $("#ledR_intensity").val();

	minTemperatureG = $("#ledG_min_temp").val();
	maxTemperatureG = $("#ledG_max_temp").val();
	intensityG = $("#ledG_intensity").val();

	minTemperatureB = $("#ledB_min_temp").val();
	maxTemperatureB = $("#ledB_max_temp").val();
	intensityB = $("#ledB_intensity").val();

	// Create an object to hold the data to be sent in the request body
	var requestData = {
	  'minTemperatureR': minTemperatureR,
	  'maxTemperatureR': maxTemperatureR,
	  'intensityR':intensityR,

	  'minTemperatureG':minTemperatureG,
	  'maxTemperatureG':maxTemperatureG,
	  'intensityG':intensityG,

	  'minTemperatureB':minTemperatureB,
	  'maxTemperatureB':maxTemperatureB,
	  'intensityB':intensityB
	};
	
	// Serialize the data object to JSON
	var requestDataJSON = JSON.stringify(requestData);
	
	$.ajax({
	  url: '/ledRGBControl.json',
	  dataType: 'json',
	  method: 'POST',
	  cache: false,
	  data: requestDataJSON, // Send the JSON data in the request body
	  contentType: 'application/json', // Set the content type to JSON
	  success: function(response) {
		// Handle the success response from the server
		console.log(response);
	  },
	  error: function(xhr, status, error) {
		// Handle errors
		console.error(xhr.responseText);
	  }
	});
}

//Control buzzer
function sendConfigBuzzer()
{
	Buzzer_min_temp = $("#Buzzer_min_temp").val();
	Buzzer_max_temp = $("#Buzzer_max_temp").val();
	
	// Create an object to hold the data to be sent in the request body
	var requestData = {
	  'Buzzer_min_temp': Buzzer_min_temp,
	  'Buzzer_max_temp': Buzzer_max_temp,
	
	};
	
	// Serialize the data object to JSON
	var requestDataJSON = JSON.stringify(requestData);
	
	$.ajax({
	  url: '/BuzzerControl.json',
	  dataType: 'json',
	  method: 'POST',
	  cache: false,
	  data: requestDataJSON, // Send the JSON data in the request body
	  contentType: 'application/json', // Set the content type to JSON
	  success: function(response) {
		// Handle the success response from the server
		console.log(response);
	  },
	  error: function(xhr, status, error) {
		// Handle errors
		console.error(xhr.responseText);
	  }
	});
}
//Control ventilador
function sendConfigFan()
{
	Fan_min_temp = $("#Fan_min_temp").val();
	Fan_max_temp = $("#Fan_max_temp").val();
	
	// Create an object to hold the data to be sent in the request body
	var requestData = {
	  'Fan_min_temp': Fan_min_temp,
	  'Fan_max_temp': Fan_max_temp,
	
	};
	
	// Serialize the data object to JSON
	var requestDataJSON = JSON.stringify(requestData);
	
	$.ajax({
	  url: '/FanControl.json',
	  dataType: 'json',
	  method: 'POST',
	  cache: false,
	  data: requestDataJSON, // Send the JSON data in the request body
	  contentType: 'application/json', // Set the content type to JSON
	  success: function(response) {
		// Handle the success response from the server
		console.log(response);
	  },
	  error: function(xhr, status, error) {
		// Handle errors
		console.error(xhr.responseText);
	  }
	});
}

function setDefault()
{
	$.ajax({
		url: '/set_as_default.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

}

/* function increaseBrightness() 
{	
	$.ajax({
		url: '/pwmUP.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
	});

} */















    










    


