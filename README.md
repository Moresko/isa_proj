Autor : Martin Mores
Dátum : 15.11.2023


Odovzdané súbory:
-----------
- tftp-server.c
- tftp-client.c
- Makefile
- README.md


Príklad spustenia
------------
Aplikaciu spustime pomocou príkazu make all, nasledne

upload:
./tftp-client -h 1234 -p 8080 -t menoSubora.txt
./tftp-server -p 8080 /Users/Documents/school/  

upload súbpra prebehne na adresu /Users/Documents/school/menoSubora.txt 

pri uploade klient vyberie meno ako sa má súbor volať pomocou parametru -t a nasledne po spustení aplikácie tftp.client bude klient pomocou stdin môcť nahrať obsah súbora. Potom keď sa tam napíše čo chceme, stlačíme enter a nasledne hneď ctrl + D 

download:
./tftp-client -h 1234 -p 8080 -f /Users/martinmores/Documents/school/ISA/test.txt -t menosSubora.txt
./tftp-server -p 8080 /Users/Documents/school/ 

tu sa subor z parametra -f stiahne na /Users/Documents/school/menosSubora.txt

Errory možu nastať buď keď súbor už na ceste existuje a snažíme sa ho prepísať alebo keď stahujeme súbor ktorý neexistuje 

Krátky popis:
-----------
Program je zložený z dvoch aplikácii TFTP server a TFTP klient. Táto aplikácia pomocou protokolu RFC 1350 umožnuje upload a download súborov zo servera na klienta a obrátene 







