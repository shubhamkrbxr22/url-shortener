async function shortenURL(){

let url = document.getElementById("urlInput").value;

let response = await fetch("http://localhost:8080/shorten",{
method:"POST",
headers:{
"Content-Type":"application/json"
},
body:JSON.stringify({url:url})
});

let data = await response.json();

let shortLink = data.short_url;

document.getElementById("shortUrl").innerText = shortLink;
document.getElementById("resultBox").classList.remove("hidden");

}

function copyURL(){

let text = document.getElementById("shortUrl").innerText;

navigator.clipboard.writeText(text);

alert("Copied to clipboard!");

}