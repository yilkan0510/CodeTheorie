# Playfair kraakscript

Deze C++ kraker vindt automatisch de Playfair-sleutel en plaintext uit `02-OPGAVE-playfair.txt` met simulated annealing en quadgram-scores.

## Hoe runnen
- Vereist: `g++` (C++17).
- Compileer en run **in de map `playfair/`** (basePath is `../`):
  ```bash
  cd playfair
  g++ -std=c++17 main.cpp -O3 -o playfair_cracker
  ./playfair_cracker
  ```
- Output (sleutel + plaintext) wordt geschreven naar `playfair/decrypted_solution.txt`.

## Gebruikte logica (stappenplan)
1) **Preprocess input**: lees ciphertext, filter op letters, maak alles uppercase, vervang `J`→`I`, en zorg dat het aantal karakters even is (laatste char droppen indien nodig).
2) **Laad taalmodel**: lees Spaanse quadgram-frequenties en bouw een scorer die een log-likelihood/score teruggeeft.
3) **Initialiseer sleutel**: begin vanaf een (goede) startkey (`YTVWXIGABRQELCMHUZDFSKNOP`) voor het 5x5 Playfair-rooster (I/J samengevoegd). Origineel begonnen met (`ABCDEFGHIKLMNOPQRSTUVWXYZ`).
4) **Simulated annealing loop**:
   - Mutaties: ofwel twee letters swappen, of een substring omdraaien.
   - Scoren: decrypt met de kandidaat-sleutel en bereken de quadgram-score.
   - Acceptatie: altijd beter, soms slechter als `exp(delta/temperature)` hoger is dan een random kans.
   - Koeling/stagnatie: verlaag temperatuur elke iteratie; reset stagnatie-teller bij een nieuw record.
5) **Beste resultaat bijhouden**: elke keer dat de score verbetert, log het (iteratie, temperatuur, sleutel, snippet) en onthoud sleutel + plaintext.
6) **Schrijf uitkomst**: na de loop sleutel, score en volledige plaintext wegschrijven naar `playfair/decrypted_solution.txt`.

## Gevonden oplossing
- Sleutel: `YTVWXIGABRQELCMHUZDFSKNOP`
- Beste score: `-10527.9`
- Plaintext: opgeslagen in `playfair/decrypted_solution.txt`.
