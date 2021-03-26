const button = document.getElementById("compile-run");


//Metoden venter på at Compile and run metoden skal trykkes på
button.addEventListener('click', function()
{       
    const Http = new XMLHttpRequest(); //Oppretter et http-request object
    const URL = "http://localhost:8080/compile"; //Setter URL til backend
    Http.open("POST", URL); //Åpner for en post melding på URL'en
    Http.send(encodeURIComponent(document.getElementById("input-field").value)); //Sender det som står i textarea i html
    document.getElementById("output-text").innerText = "Compiling...";
    Http.onreadystatechange = function ()
    {
        document.getElementById("output-text").innerText = Http.responseText; //Sett output teksten til svaret fra backend
    }
});
