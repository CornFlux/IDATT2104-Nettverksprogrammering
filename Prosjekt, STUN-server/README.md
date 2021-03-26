Nettverksprogrammering (IDATT2104) Prosjekt.

Laget av Oline Amundsen og Ola Kristoffer Hoff.

Videoserien til Javidx9 på YouTube var til god hjelp til å sette seg inn i ASIO biblioteket https://www.youtube.com/watch?v=2hNdkYInj4g

Det som blir omtalt som 'protokollen' nedenfor er RFC5389 https://tools.ietf.org/html/rfc5389

Navnet på STUN-serveren er «stun:192.168.0.125:8000» dette er IP adressen og porten serveren er satt til. Disse må settes opp til riktige verdier for den endelige 'stedet' serveren lir satt opp, disse er per nå bare IP adressen til Wi-Fi kortet på Mac'en min og en tilfeldig port.

STUN-serveren er skrevet i C++. Den er skrevet så den lett kan utvides/videre utvikles.

Funksjonalitet:

Serveren kan motta UDP IPv4 pakker.

Den kan validere at mottatte pakker er STUN-request-message, og den kan svare med STUN-respons-message på IPv4.

Logikk for håndtering av IPv6 pakker er på plass da denne koden ble skrevet generelt for å fungere nesten likt på IPv4 og IPv6. (Mangler bare å kunne motta IPv6 pakker)

Serveren har et implementert cache, som lagrer nylig sendte 'respons' med en tilknyttet transaction ID, om klienten ikke får svaret vil den sende en ny pakke (en retransmisson med samme transaction ID) serveren vil da kunne sende svaret på nytt uten å måtte gjøre alle sjekker og utregninger på nytt. Dette cachet er ikke implemnetert optimalt og bruker en simple FIFO algoritme hvilket entry som skal byttes ut, da denne serveren ikke er tenkt å ha så stor trafikk (ikke nå hvertfall).

STUN-Attributes (implementert er markert [X]):
    MAPPED-ADDRESS [X]
    XOR-MAPPED-ADDRESS [X]
    USERNAME
    MESSAGE-INTEGRITY
    FINGERPRINT
    ERROR-CODE [X]
    REALM
    NONCE
    UNKNOWN-ATTRIBUTES
    SOFTWARE
    ALTERNATE-SERVER

Mangler:

Serveren kan ikke motta pakker på IPv6 over UDP, IPv4 over TCP og IPv6 over TCP.

Serveren kan ikke sende Error Response eller indication.

Serveren håndterer ikke pakker (requests) med attributes.

Serveren håndterer ikke 'indication' pakker.

Serveren implementerer ikke FINGERPRINT makansimer.

Serveren har ikke et domainname og kan derav ikke bli funnet gjennom DNS. Må vite serveren sin IP adresse (og port).

'Authentication and Message-Integrity Mechanisms' er ikke implementert.

Det mangler STUN-attributes (se liste over), men koden er skrevet på en måte som tilater at det lettere skal kunne legges inn flere.

Serveren har ikke bitt sjekket opp mot 'Security Considerations', 'IAB Considerations', eller 'IANA Considerations' i protokollen


Fremtidig arbeid:

Utvid serveren til å kunne motta alle pakker som er listet som mangler, og legg inn håndtering av disse. (loggikken er på plass, må bare kobles om litt)
Legg inn resterene STUN-attributes.
Implementer håndtering av attributes i en request.
Sjekke serveren opp mot 'Authentication and Message-Integrity Mechanisms', 'Security Considerations', 'IAB Considerations' og 'IANA Considerations' i protokollen.


Eksterne avhengigheter:

ASIO biblioteket er brukt for håndtering av sockets og da motak og sending av pakkene (https://think-async.com/Asio/)


Installasjonsinstruksjoner:

Last ned 'STUN-server/src'-mappen.
Last ned og legg til eksterne avhengigheter (følg deres dokumentasjon)

Mer trenger du ikke installere.


For å starte STUN-serveren:

MacOS:
For å starte serveren gå i terminalen og 'cd' inn til 'src'-mappen hvor filene du lastet ned ligger.
Så kjører du denne kommandoen som kompilerer og kjører koden. "g++ main.cpp -o main -pthreads -std=c++1y && ./main"

Andre operativsystem vil ha tilsvarende kommandoer


Tester:

Det er ikke laget noen tester for serveren.
Det er "test linjer" i koden som er kommentert ut, men brukt under utvikling for å sjekke at verdier er som forventet.
Det at servern svarer på 'request'-pakker kan sjekkes ved bruk av f.eks. WireShark.

API:

Det er ikke noe pen nettside med en god oversikt for API'et dessverre.
Det er derimot ganske mange kommentarer i koden som kanskje vil hjelpe på forståelsen av hva som foregår.