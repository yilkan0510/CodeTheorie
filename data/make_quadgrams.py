import re
from collections import defaultdict

# --- CONFIGURATIE ---
input_file = 'corpus.txt'
output_file = 'spaceless_english_quadgrams.txt'
print(f"Starten... Corpus wordt geladen van: {input_file}")

# Lees het corpus en maak het schoon
try:
    with open(input_file, 'r', encoding='utf-8') as f:
        full_text = f.read()
except FileNotFoundError:
    print(f"FOUT: Het bestand '{input_file}' niet gevonden.")
    exit()

print("Corpus geladen. Tekst wordt nu opgeschoond...")
# Verwijder alles wat geen letter is en zet om naar hoofdletters
clean_text = re.sub(r'[^A-Z]', '', full_text.upper())
print(f"Tekst opgeschoond. Lengte: {len(clean_text)} karakters.")

# Tel de quadgrams
print("Quadgrams tellen...")
quadgram_counts = defaultdict(int)
for i in range(len(clean_text) - 3):
    quadgram = clean_text[i:i+4]
    quadgram_counts[quadgram] += 1
print(f"Tellen voltooid. {len(quadgram_counts)} unieke quadgrams gevonden.")

# Schrijf de output weg
print(f"Resultaten wegschrijven naar: {output_file}")
with open(output_file, 'w', encoding='utf-8') as f:
    for quadgram, count in quadgram_counts.items():
        f.write(f"{quadgram} {count}\n")

print("\nKlaar! Je nieuwe quadgram-bestand is succesvol aangemaakt.")