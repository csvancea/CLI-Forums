#  Tema 2 - client-server
## Student: Cosmin-Razvan VANCEA - 323CA
----------------------------------------


### Descrierea protocolului:

#### Client UDP -> Server UDP
* `UDPServer` porneste un socket UDP si asculta pe el.
* Cand ajunge un packet, il plasez intr-un `BitStream` (explicatie mai jos)
si il dau mai departe catre `Server`
* In `Server` verific daca pachetul contine topicul, tipul si inca x bytes,
reprezentand lungimea tipului care ar trebui sa fie in packet
* Daca totul este ok, citesc datele din `BitStream` si salvez mesajul
in `Forums` (explicatie mai jos)

#### Client TCP <-> Server TCP
* Comunicatia TCP se face exclusiv pe BitStream-uri.
* Toate datele trimise pe TCP sunt BitStream-uri.
* Inainte de a se face trimiterea mesajului (inainte de `send` syscall)
se adauga la inceputul BitStream-ului lungimea acestuia in bytes (max 2^16-1)
* La receptie, se salveaza tot ce a venit pe socketul TCP si se reasambleaza
mesajele in `BitStream`-uri, asa cum au fost ele la sursa. De exemplu daca:
  - un `BitStream` a fost fragmentat in mai multe bucati, atunci acesta este mai
intai 'lipit' si dupa este dat mai departe catre Server/Client pentru procesare
  - mai multe `BitStream`uri au fost 'lipite' de TCP, atunci acestea sunt mai intai
separate (si poate completate la urmatorul `recv` daca ultimul lipit nu este intreg)
si dupa date mai departe catre Server/Client pentru procesare
* Aceasta reasamblare se face in functie de size-ul care este adaugat inaintea
fiecarui BitStream.

* Toate BitStream-urile care se transmit intre client-server pe TCP trebuie
sa inceapa cu un cod (RPC) care arata ce este acel pachet si care este
structura lui. Ce urmeaza dupa parametrul RPC depinde de valoarea RPC-ului
(ex: `RPC_UNSUBSCRIBE` este urmat de o serializare a `std::string` ce
reprezinta numele topicului de la care se dezaboneaza clientul)

* Lista curenta de RPC-uri:
  * RPC_MESSAGE: serverul trimite catre client un mesaj venit de la UDP
  * RPC_ANNOUNCE: clientul isi anunta client_id-ul catre server
  * RPC_SUBSCRIBE: clientul vrea sa se aboneze la un topic
  * RPC_UNSUBSCRIBE: clientul vrea sa se dezaboneze de la un topic

#### Salvarea mesajelor
* La sosirea unui mesaj pe UDP:
  1. este pasat catre `Forums`
  2. `Forums` numara clientii care sunt abonati la topic si:
     + fie sunt online
     + fie sunt offline, dar au SF=1
  3. Salveaza aceasta numaratoare ca `ref_count`
  4. Ii asociaza mesajului un ID incremental (mesajele nou venite au obligatoriu
  ID-ul mai mare decat al oricarui mesaj din trecut)

* Dupa procesarea mesajelor UDP in tick-ul curent, se trece la procesarea mesajelor
stocate in `Forums`:
  1. Serverul cere forumului -pentru fiecare client conectat- o lista a mesajelor care trebuie
  trimise acestuia
  2. Forumul parseaza toate topicurile la care este abonat clientul respectiv, iar cand
  gaseste un mesaj netrimis (message_id > client_last_message_id) il adauga intr-o lista
  si scade `ref_count` cu 1.
  3. Cand `ref_count` ajunge la 0 pentru un mesaj inseamna ca acel mesaj a fost trimis
  tututor clientilor care erau la momentul venirii mesajului abonati la topic si in
  consecinta mesajul poate fi sters din memorie.
  4. Serverul trimite toate mesajele returnate de forum catre clientul in cauza


### Module:
#### Common:
Aceste module sunt folosite atat de server, cat si de client(subscriber):

1. BitStream
  * Reprezinta un stream de bytes.
  * Abstractizeaza scrierea si citirea datelor astfel incat sa nu depinda
  de endianness-ul clientului sau serverului.
  * Este folosit si la serializarea structurilor custom care urmeaza a fi
  trimise pe retea (ex: SHORTREAL, FLOAT etc - vezi `NetworkObjects.cpp`)
  * Comunicarea client-server foloseste exclusiv BitStream-uri.

2. ISelectable
  * Este o interfata.
  * Toate clasele ce o implementeaza pot fi `select`-ate.
  * Implementari: `Keyboard`, `TCPServer`, `TCPClient`, `UDPServer`

3. Selector
  * Ia o multime de obiecte `ISelectable` si asteapta date pentru oricare
  obiect (file descriptor).
  * Cand primeste date (syscallul `select` returneaza), da controlul
  obiectului care a fost selectat pentru a citi datele tocmai venite.

4. Keyboard
  * Aceasta clasa are rolul de a citi de la tastatura comenzi si de a le
  plasa intr-o coada pana vor urma a fi procesate de server/client.

5. NetworkObjects
  * Contine toate clasele custom ce pot fi serializate si trimise pe retea,
  impreuna cu codul care face serializarea. (ex: `STRING`, `SHORTREAL` etc)

#### Server:

1. UDPServer
  * Deschide un socket UDP si asteapta date pe el de la clienti.
  * Plaseaza mesajele intr-o coada si urmeaza a fi procesate de server.

2. TCPServer
  * Deschide un socket TCP si asteapta pe el conexiuni.
  * Cand vine o noua conexiune, creaza o instanta a `TCPClient` asociata
  noii conexiuni.

3. TCPClient
  * Se ocupa de comunicarea intre serverul TCP si un anumit client TCP.
  * Primeste/trimite mesaje de la/catre clientul TCP asociat.
  * Mesajele primite le da mai departe catre `TCPServer` care le trimite
  mai departe catre `Server`

4. Forums
  * Se ocupa de structurarea si memorarea mesajelor UDP.
  * Tine cont ce mesaje a primit fiecare client si ce mesaje trebuie sa
  primeasca in continuare.

4. Server
  * Logica principala a serverului.
  * Administreaza clientii TCP, serverele TCP/UDP si mesajele.

#### Client:

1. TCPClient
  * Stabileste conexiunea cu serverul TCP si trimite/primeste mesaje.

2. Client
  * Logica clientului.
  * Interpreteaza mesajele primite si da comenzi serverului (subscribe etc)
