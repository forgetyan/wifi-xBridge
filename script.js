function OpenSSIDPopup(ssid)
{
	var popup = document.getElementById("popup");
	var ssid_name = document.getElementById("ssid_name");
	var ssid_name_text = document.getElementById("ssid_name_text");
	var ssid_password = document.getElementById("ssid_password");
	ssid_name.value = ssid;
	ssid_name_text.innerHTML = ssid;
	ssid_password.value = "";
	popup.style.visibility = "visible";
	popup.style.opacity = 1;
}

function SaveSSID() {
	var ssid_name = document.getElementById("ssid_name");
	var ssid_password = document.getElementById("ssid_password");
	var frmSaveSSID = document.getElementById("frmSaveSSID");
	frmSaveSSID.submit();
}

function ClosePopup() {
	var popup = document.getElementById("popup");
	popup.style.visibility = "hidden";
	popup.style.opacity = 0;
}

function RemoveSSID(ssid)
{
	if (confirm("Do you really want to remove " + ssid)) {
		document.location.href='remove/' + ssid;
	}
}

function TestSSID(ssid) {
	var xhttp = new XMLHttpRequest();
	xhttp.open("GET", "test/" + ssid, true);
	xhttp.onreadystatechange = function () {
		if(xhttp.readyState === XMLHttpRequest.DONEDONE && xhttp.status === 200){
			alert(xhttp.responseText);
			console.log(xhttp.responseText);
		};
	};
	xhttp.send();
}


function ScanWifi() {
	var xhttp = new XMLHttpRequest();
	xhttp.open("GET", "scannedWifi", true);
	xhttp.onreadystatechange = function () {
		if(xhttp.readyState === XMLHttpRequest.DONEDONE && xhttp.status === 200){
			var divScannedWifi = document.getElementById("scannedWifi");
			divScannedWifi.innerHTML = xhttp.responseText;
		};
	};
	xhttp.send();
	
	
}
