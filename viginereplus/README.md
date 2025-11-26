# Vigenere-plus kraakscript

Dit C++ programma zoekt de oplossing van de Vigenère-plus-opgave uit `01-OPGAVE-viginereplus.txt`.

## Hoe runnen
- Vereist: `g++` (C++17 of hoger).
- Compileer en run vanuit de root van deze map:
  ```bash
  g++ -std=c++17 -O3 vigenereplus_solver.cpp -o vigenereplus_solver
  ./vigenere_solver
  ```
- Het script leest automatisch de ciphertext uit `01-OPGAVE-viginereplus.txt`. Wil je een ander bestand gebruiken, pas het pad in `main` aan.
- Runtime: totale run duurt enkele minuten, maar al na een paar seconden verschijnt de juiste plaintext bij transpositie breedte 6.

## Gebruikte logica (stappenplan)
1) **Inlezen**: lees het ciphertextbestand direct in.
2) **Transpositie terugdraaien**: veronderstel een kolomtranspositie met breedtes 2 t/m 10. Voor elke breedte test het programma alle permutaties en reconstrueert de tekst rij-voor-rij (`decrypt_columnar`).
3) **Vigenère-geschiktheid scoren**: voor elke transpositie-kandidaat bereken we de gemiddelde Index of Coincidence van periodieke slices voor sleutel-lengtes 1–10. Alleen kandidaten met een hoge IC (≈0.077 verwacht voor NL-tekst) gaan door (`score_vigenere_potential`).
4) **Beste Vigenère-sleutellengte vinden**: bepaal voor de overgebleven kandidaat de sleutel-lengte met hoogste gemiddelde IC per kolom.
5) **Sleutel per kolom bepalen**: voer per kolom een Caesar-verschoven frequentie-analyse uit met Nederlandse letterfrequenties; kies de shift met laagste chi-kwadraat (`solve_vigenere`).
6) **Decryptie en output**: decrypt met de gevonden sleutel, print sleutel + plaintext voor elke gevonden hit, samen met de gebruikte transpositiepermutatie.

## Gevonden Oplossing
- Transpositie Breedte: 6
- Transpositie Permutatie: 3 0 2 4 1 5
- IC Score: 0.0827743
- Key: `HPEGSJFE`
- Plaintext:

```
DEKLEINEERIKLAGJUISTOPHETOGENBLIKDATDITBOEKJEBEGINTINHETOUDEBEDVANGROOTMOEDERPINKSTERBLOMMETDETROONHEMELENDEZIJDENKWASTENENKEEKOVERDERANDVANHETBLANKELAKENDESCHEMERIGEKAMERINHETWASHETUURWAAROPDEKLEINEMENSENNAARBEDGAANHETUURWAARDEGROTEMENSENNIETVANWETENALLEVERTROUWDEDINGENVANDEMUURVERVAGENZOETJESAANINHETGROEIENDEDUISTERENDEWERELDWORDTSTILZOSTILDATZIJZELFSNIETMEERADEMTBUITENSTAPTNOGIEMANDVOORBIJSTAPSTAPZOKLINKTHETENINDEVERTEROEPTEENJONGETJEHOOGENFIJNNAAREENANDERJONGETJEZIJNSTEMKLINKTINDEAVONDENJEDENKTDAARISTOCHEENJONGETJEOPDEWERELDDATNOGNIETINBEDLIGTERIKLAGSTILTEKIJKENNAARHETRAAMINDEVERTEENNAARDESCHEMERENDEPORTRETTENVANDEMUURHETISNETDACHTHIJOFERIETSGEBEURENGAATENMISSCHIENGAATEROOKWELIETSGEBEURENENHIJBESLOOTOMNUEENSNIETGELIJKOPANDEREAVONDENINSLAAPTEVALLENMAARGOEDOPTELETTENOFERMISSCHIENIETSGEBEURENGINGNUWASDAAREENGOEDMIDDELVOORWANTONDERZIJNHOOFDKUSSENLAGEENBOEKJESOLMSBEKNOPTENATUURLIJKEHISTORIEGEHETENENERIKMOESTDAARVOORMORGENALLEINSECTENUITKENNENHIJHADERDEZEHELEWOENSDAGMIDDAGUITZITTENLERENENWASTOTAANDEMEIKEVERSGEKOMENMORGENOCHTENDONDERHETSPEELKWARTIERZOUHIJDEMEIKEVERSERBIJNEMENLAATEENSKIJKENMOMPELDEERIKHOEVEELPOTENHEEFTEENWESPOOKALWEERZESDEOGENZIJNAPARTVERSTELBAARENSTAANVOORINDEKOPMOOIZIJLEVENNIETINKORVENGELIJKDEBIJENMAARJAWAARLEVENZIJDANZIJZULLENAPARTLEVENDENKIKNUDATDOETEROOKNIETTOEZIJBEHORENTOTDEFAMILIEDERVLIESVLEUGELIGENENHEBBENGEKNIKTESPRIETENENHOESTAATHETMETDEVLINDERS
```



