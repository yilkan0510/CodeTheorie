// playfair/main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <filesystem> // C++17, voor paden

#include "QuadgramScorer.h"
#include "Playfair.h"

// Functie om de ciphertext uit het bestand te lezen en voor te bereiden
std::string loadAndPrepareCiphertext(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Kon ciphertext-bestand niet openen: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    // Verwijder alles wat geen letter is en zet om naar hoofdletters
    std::string cleaned_text;
    for (char c : text) {
        if (isalpha(c)) {
            cleaned_text += toupper(c);
        }
    }

    // Vervang J door I
    std::replace(cleaned_text.begin(), cleaned_text.end(), 'J', 'I');

    return cleaned_text;
}


int main() {
    try {
        // --- CONFIGURATIE ---
        std::filesystem::path basePath = "../../"; // Pas dit aan indien nodig
        std::string ciphertext_path = (basePath / "playfair" / "test_input.txt").string();
        std::string quadgrams_path = (basePath / "data" / "english_quadgrams.txt").string();
        const int MAX_ITERATIONS = 1000000;

        // --- INITIALISATIE ---
        std::cout << "Laden van quadgrams van: " << quadgrams_path << std::endl;
        QuadgramScorer scorer(quadgrams_path);

        std::cout << "Laden van ciphertext van: " << ciphertext_path << std::endl;
        std::string ciphertext = loadAndPrepareCiphertext(ciphertext_path);
        std::cout << "Ciphertext geladen (" << ciphertext.length() << " tekens)." << std::endl;

        Playfair cipher;

        // Willekeurige generator voor sleutelmutaties
        std::mt19937 rng(std::random_device{}());

        // Genereer een willekeurige startsleutel
        std::string parent_key = "ABCDEFGHIKLMNOPQRSTUVWXYZ";
        std::shuffle(parent_key.begin(), parent_key.end(), rng);

        cipher.setKey(parent_key);
        double parent_score = scorer.score(cipher.decrypt(ciphertext));

        std::string best_key = parent_key;
        double best_score = parent_score;

        // --- HILL-CLIMBING LOOP ---
        std::cout << "\nStarten van de hill-climbing aanval..." << std::endl;
        for (int i = 0; i < MAX_ITERATIONS; ++i) {
            std::string child_key = parent_key;

            // Muteer de sleutel: wissel twee willekeurige letters
            std::uniform_int_distribution<int> dist(0, 24);
            std::swap(child_key[dist(rng)], child_key[dist(rng)]);

            cipher.setKey(child_key);
            double child_score = scorer.score(cipher.decrypt(ciphertext));

            // Als de nieuwe sleutel beter is, wordt het de nieuwe "parent"
            if (child_score > parent_score) {
                parent_score = child_score;
                parent_key = child_key;
            }

            // Hou de allerbeste sleutel ooit gevonden bij
            if (parent_score > best_score) {
                best_score = parent_score;
                best_key = parent_key;

                std::cout << "Iteratie " << i << " | Beste Score: " << best_score << std::endl;
                std::cout << "Huidige tekst: " << cipher.decrypt(ciphertext).substr(0, 100) << "..." << std::endl;
                std::cout << "Huidige sleutel: " << best_key << std::endl << std::endl;
            }
        }

        // --- RESULTAAT ---
        std::cout << "\n--- Aanval voltooid ---" << std::endl;
        std::cout << "Beste gevonden sleutel: " << best_key << std::endl;
        std::cout << "Beste score: " << best_score << std::endl;

        cipher.setKey(best_key);
        std::cout << "Ontsleutelde tekst:" << std::endl;
        std::cout << cipher.decrypt(ciphertext) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}