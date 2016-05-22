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
	//document.location.href='add/' + ssid_name.value;
	//alert(ssid_password.value);
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
	var xhttp;
	if (window.XMLHttpRequest) {
        // code for IE7+, Firefox, Chrome, Opera, Safari
        xhttp = new XMLHttpRequest();
    } else {
        // code for IE6, IE5
        xhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
	xhttp.open("GET", "test/" + ssid, true);
	xhttp.send();
	alert(xhttp.responseText);
}