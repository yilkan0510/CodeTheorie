# Playfair Cipher Cracker

Dit project is een C++ implementatie van een automatische kraker voor het Playfair-cijfer. Het maakt gebruik van een *simulated annealing* algoritme, geleid door quadgram-statistieken, om de meest waarschijnlijke sleutel voor een gegeven ciphertext te vinden.

## Inhoudsopgave
1. [Hoe Werkt Het?](#hoe-werkt-het)
    - [Het Playfair Cijfer](#het-playfair-cijfer)
    - [De Kraakmethode: Statistische Analyse](#de-kraakmethode-statistische-analyse)
    - [Optimalisatie: Simulated Annealing](#optimalisatie-simulated-annealing)
2. [Projectstructuur](#projectstructuur)
3. [Hoe te Gebruiken](#hoe-te-gebruiken)
    - [Vereisten](#vereisten)
    - [Compilatie & Uitvoering](#compilatie--uitvoering)
4. [Resultaten](#resultaten)
    - [Gevonden Sleutel](#gevonden-sleutel)
    - [Ontsleutelde Tekst](#ontsleutelde-tekst)

## Hoe Werkt Het?

### Het Playfair Cijfer

Het Playfair-cijfer is een digrafische substitutieversleuteling, wat betekent dat het letters in paren versleutelt in plaats van individueel. De kern van het cijfer is een 5x5 rooster, gevuld met de letters van een sleutelwoord, gevolgd door de resterende letters van het alfabet. In deze implementatie wordt de 'J' samengevoegd met de 'I' om de 25 letters in het rooster te passen.

De ontsleuteling volgt drie simpele regels, gebaseerd op de posities van de letterparen in het rooster:
1.  **Zelfde Rij**: Als beide letters in dezelfde rij staan, worden ze vervangen door de letter direct links van hen (met een terugloop aan het begin van de rij).
2.  **Zelfde Kolom**: Als beide letters in dezelfde kolom staan, worden ze vervangen door de letter direct boven hen (met een terugloop aan de top van de kolom).
3.  **Rechthoek**: Als de letters een rechthoek vormen, worden ze vervangen door de letter op dezelfde rij, maar in de andere hoek van de rechthoek.

### De Kraakmethode: Statistische Analyse

Het aantal mogelijke sleutels voor een 5x5 Playfair rooster is 25! (faculteit van 25), wat een astronomisch groot getal is. Een brute-force aanval is dus onmogelijk. Dit programma gebruikt een intelligentere aanpak gebaseerd op statistische analyse.

Elke natuurlijke taal (zoals Engels, Frans of Spaans) heeft een voorspelbare frequentie van lettercombinaties. Vier-letter combinaties, zogenaamde **quadgrams**, zijn hiervoor zeer effectief. De quadgram "CIÓN" komt bijvoorbeeld veel vaker voor in het Spaans dan "QXZJ".

De `QuadgramScorer.h` klasse doet het volgende:
1.  Laadt een referentiebestand met quadgram-frequenties (in dit geval voor het Spaans).
2.  Berekent een "fitness score" voor een gegeven (ontsleutelde) tekst.
3.  Hoe hoger de score, hoe meer de tekst statistisch gezien lijkt op de Spaanse taal.

### Optimalisatie: Simulated Annealing

De kern van de kraker is een "hill-climbing" algoritme met een belangrijke verbetering: **Simulated Annealing**.

1.  **Start**: Het programma begint met een willekeurige of vooraf ingestelde sleutel (`YTVWXIGABRQELCMHUZDFSKNOP` in de laatste run).
2.  **Iteratie**: In elke stap wordt een kleine, willekeurige wijziging aan de sleutel gemaakt (bv. twee letters omwisselen).
3.  **Evaluatie**: De ciphertext wordt ontsleuteld met deze nieuwe "kind"-sleutel en de fitness score wordt berekend.
4.  **Beslissing**:
    *   Als de nieuwe score *beter* is dan de vorige, wordt de nieuwe sleutel altijd geaccepteerd.
    *   Als de nieuwe score *slechter* is, kan de nieuwe sleutel *soms toch geaccepteerd worden*. De kans hierop hangt af van een "temperatuur"-parameter.

Dit laatste punt is cruciaal. Een simpel "hill-climbing" algoritme zou altijd de beste score kiezen en kan vast komen te zitten in een "lokaal optimum" (een sleutel die goed is, maar niet de beste). Door af en toe een slechtere oplossing te accepteren (vooral bij een hoge "temperatuur" aan het begin), kan het algoritme ontsnappen aan deze lokale optima en een groter deel van de oplossingsruimte verkennen. Naarmate het proces vordert, "koelt" de temperatuur af, waardoor het algoritme zich stabiliseert rond een zeer goede, waarschijnlijk de correcte, oplossing.

## Projectstructuur

Het project is opgedeeld in drie hoofdbestanden:

-   `main.cpp`: Het hoofdbestand dat de simulated annealing loop aanstuurt. Het laadt de data, initialiseert de kraker, voert de iteraties uit en slaat het uiteindelijke resultaat op.
-   `Playfair.h`: Bevat de `Playfair` klasse. Deze klasse is verantwoordelijk voor het aanmaken van het 5x5 rooster op basis van een sleutel en het ontsleutelen van de ciphertext volgens de Playfair-regels.
-   `QuadgramScorer.h`: Bevat de `QuadgramScorer` klasse. Deze laadt de taalstatistieken en berekent de fitness score van een stuk tekst.

## Hoe te Gebruiken

### Vereisten
*   Een C++ compiler met ondersteuning voor C++17 (vereist voor `std::filesystem`).

### Compilatie & Uitvoering

1.  Zorg ervoor dat de mappenstructuur correct is (`/data`, `/playfair`).
2.  Compileer de code. Bijvoorbeeld met g++:
    ```bash
    g++ -std=c++17 playfair/main.cpp -o playfair_cracker
    ```
3.  Voer het programma uit:
    ```bash
    ./playfair_cracker
    ```

Het programma laadt automatisch de bestanden, start het kraakproces en slaat de uiteindelijke oplossing op in `playfair/decrypted_solution.txt`.

## Resultaten

Na ongeveer 3.000.000 iteraties heeft het algoritme de volgende oplossing gevonden:

### Gevonden Sleutel

YTVWXIGABRQELCMHUZDFSKNOP

### Beste Score

-10527.9