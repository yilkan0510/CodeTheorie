# ADFGVX kraakscript

Kraker voor de ADFGVX-opgave (`03-OPGAVE-adfgvx.txt`). Stap 1 brute-force’t de transpositie-sleutel, stap 2 zoekt het beste Polybius-vierkant. Morsecode wordt eerst naar A/D/F/G/V/X omgezet.

## Hoe runnen (zonder CMake, vanuit `adfgvx/`)
- Vereist: `g++` (C++17); quadgrams uit `../data/spaceless_english_quadgrams.txt`.
- Compileer en run **in de map `adfgvx/`** (basePath is `../`):
  ```bash
  cd adfgvx
  g++ -std=c++17 -O3 find_transposition.cpp -o find_transposition_adfgvx
  g++ -std=c++17 -O3 solve_square.cpp -o solve_square_adfgvx
  ./find_transposition_adfgvx   # fase 1: zoek transpositie
  ./solve_square_adfgvx        # fase 2: zoek Polybius-vierkant
  ```
- Output/logs komen in `adfgvx/result.txt` (relatief t.o.v. repo-root).

## Gebruikte logica (stappenplan)
1) **Inlezen & morse-prep**: laad de morse-cipher, verwijder alle whitespace, parse morse-sequenties tot A/D/F/G/V/X (dropt onbekende tokens).
2) **Basisvalidatie**: zorg dat de lengte even is (laatste char droppen indien oneven), want ADFGVX decode werkt per digraf.
3) **Taalmodel laden**: laad Engelse quadgram-frequenties en bouw twee scorers: tolerant (fase 1) en strikt (fase 2).
4) **Fase 1 – transpositie brute-force** (`find_transposition_adfgvx`):
   - Genereer alle kolom-permutaties voor de veronderstelde transpositie-breedte.
   - Voor elke permutatie: start met een geschud 6x6 Polybius-square (letters+digits) en voer simulated annealing uit (20k iteraties).
   - Mutaties: swap twee posities in het square; acceptatie via scoreverschil/temperatuur.
   - Score: decrypt met die permutatie+square en evalueer met quadgrams (tolerant). Parallelle threads houden lokaal en globaal beste scores bij en loggen voortgang.
   - Bewaar de best scorende permutatie.
5) **Fase 2 – square verfijnen** (`solve_square_adfgvx`):
   - Neem de beste transpositie-sleutel uit fase 1.
   - Zoek ~10M iteraties naar het beste Polybius-square met simulated annealing (swaps of substring-reversals), strikte quadgram-score.
   - Log periodiek iteratie/score/temp en update bij elk nieuw record de sleutel + plaintext.
6) **Resultaat opslaan**: schrijf beste transpositie, square, score en volledige plaintext naar `adfgvx/result.txt`.

## Gevonden oplossing
- Transpositie-sleutel: `4 0 6 2 1 3 5`
- Polybius-square: `W4BGVDS59KFEUR6PHMOTA0X2N7JQ13YZ8ILC`
- Beste score (fase 2): `-9400.68`
- Plaintext: Greta Thunberg “How Dare You?” speech, beginnend met:
  ```
  MYMESSAGEISTHATWELLBEWATCHINGYOU...
  ```
- Volledige log en tekst: `adfgvx/result.txt`.
