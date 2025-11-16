// solve_square.cpp - DE ECHTE FINALE VERSIE
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <filesystem>
#include <cmath>
#include <numeric>
#include <map>
#include <iomanip>

#include "QuadgramScorer.h"
#include "ADFGVX.h"

// Functies blijven hetzelfde
std::string convertMorseToADFGVX(const std::string& morse_code) {
    const std::map<std::string, char> morse_map = {{".-", 'A'}, {"-..", 'D'}, {"..-.", 'F'}, {"--.", 'G'}, {"...-", 'V'}, {"-..-", 'X'}};
    std::string adfgvx_text;
    std::stringstream ss(morse_code);
    std::string single_morse_char;
    while (std::getline(ss, single_morse_char, '/')) {
        if (morse_map.count(single_morse_char)) {
            adfgvx_text += morse_map.at(single_morse_char);
        }
    }
    return adfgvx_text;
}
std::string loadFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Kon bestand niet openen: " + filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();
    text.erase(std::remove_if(text.begin(), text.end(), ::isspace), text.end());
    return text;
}

int main() {
    try {
        std::filesystem::path basePath = "../../";
        std::string ciphertext_path = (basePath / "adfgvx" / "03-OPGAVE-adfgvx.txt").string();
        std::string quadgrams_path = (basePath / "data" / "spaceless_english_quadgrams.txt").string();

        QuadgramScorer scorer(quadgrams_path);
        std::string morse_ciphertext = loadFileContent(ciphertext_path);
        std::string ciphertext = convertMorseToADFGVX(morse_ciphertext);
        if (ciphertext.length() % 2 != 0) { ciphertext.pop_back(); }
        std::cout << "Ciphertext geladen (" << ciphertext.length() << " tekens).\n" << std::endl;

        ADFGVX cipher;
        std::mt19937 rng(std::random_device{}());
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "--- FASE 2: Starten van definitieve aanval op het Polybius-vierkant ---\n" << std::endl;

        // DE SLEUTEL DIE JOUW PROGRAMMA HEEFT GEVONDEN.
        std::vector<int> best_transpo_key = {TEMP};

        std::cout << "Gebruiken van bewezen transpositie-sleutel: ";
        for(int k : best_transpo_key) std::cout << k << " ";
        std::cout << std::endl;

        const int ITERATIONS_FOR_SQUARE_SEARCH = 5000000;

        std::string parent_square_key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::shuffle(parent_square_key.begin(), parent_square_key.end(), rng);

        cipher.setKeys(parent_square_key, best_transpo_key);
        double parent_score = scorer.score_strict(cipher.decrypt(ciphertext));

        double overall_best_score = -999999.0;
        std::string best_square_key = "";
        std::string best_plaintext = "";

        double temperature = 100.0;
        double cooling_rate = 0.999995;
        int stagnation_counter = 0;
        const int stagnation_limit = 500000;

        for (int i = 0; i < ITERATIONS_FOR_SQUARE_SEARCH; ++i) {
             if (i > 0 && i % 50000 == 0) {
                std::cout << "Iter: " << i / 1000 << "k | Huidige: " << parent_score << " | Beste: " << overall_best_score << " | Temp: " << temperature << std::endl;
            }

            std::string child_square_key = parent_square_key;
            std::uniform_int_distribution<int> mutation_choice(0, 100);
            int choice = mutation_choice(rng);

            if (choice < 60) {
                // beperking enkel letters
//                 int pos_a = -1, pos_b = -1;
//                 while (pos_a == -1 || !std::isalpha(child_square_key[pos_a])) { pos_a = std::uniform_int_distribution<int>(0, 35)(rng); }
//                 while (pos_b == -1 || pos_b == pos_a || !std::isalpha(child_square_key[pos_b])) { pos_b = std::uniform_int_distribution<int>(0, 35)(rng); }
//                 std::swap(child_square_key[pos_a], child_square_key[pos_b]);
                 int pos_a = std::uniform_int_distribution<int>(0, 35)(rng);
                 int pos_b = std::uniform_int_distribution<int>(0, 35)(rng);
                 std::swap(child_square_key[pos_a], child_square_key[pos_b]);
            } else {
                int start = std::uniform_int_distribution<int>(0, 35)(rng);
                int end = std::uniform_int_distribution<int>(0, 35)(rng);
                if (start > end) std::swap(start, end);
                std::reverse(child_square_key.begin() + start, child_square_key.begin() + end);
            }

            cipher.setKeys(child_square_key, best_transpo_key);
            std::string child_plain = cipher.decrypt(ciphertext);
            if (child_plain.empty()) continue;

            double child_score = scorer.score_strict(child_plain);
            if (child_score > parent_score || exp((child_score - parent_score) / temperature) > std::uniform_real_distribution<>(0.0, 1.0)(rng)) {
                parent_score = child_score; parent_square_key = child_square_key;
            }

            if (parent_score > overall_best_score) {
                overall_best_score = parent_score;
                best_square_key = parent_square_key;
                best_plaintext = cipher.decrypt(ciphertext);
                stagnation_counter = 0;
                std::cout << "\n>>> NIEUW RECORD <<<" << " Score: " << overall_best_score << std::endl;
                std::cout << "    Tekst: " << best_plaintext.substr(0, 80) << "...\n" << std::endl;
            } else {
                stagnation_counter++;
            }

            temperature *= cooling_rate;
            if(stagnation_counter > stagnation_limit){
                std::cout << "\n--- STAGNATIE! NUCLEAIRE RESET ---\n" << std::endl;
                temperature = 60.0; // Reset temperatuur
                stagnation_counter = 0;
                // GOOI DE HUIDIGE SLEUTEL WEG EN BEGIN VOLLEDIG OPNIEUW
                parent_square_key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                std::shuffle(parent_square_key.begin(), parent_square_key.end(), rng);
                parent_score = scorer.score_strict(cipher.decrypt(ciphertext));
            }
        }

        std::cout << "\n\n--- Aanval voltooid ---" << std::endl;
        std::cout << "Beste score: " << overall_best_score << std::endl;
        std::cout << "Beste Polybius sleutel: " << best_square_key << std::endl;
        std::cout << "Beste transpositievolgorde: ";
        for (int i : best_transpo_key) std::cout << i << " ";
        std::cout << "\n\nOntsleutelde tekst:\n" << best_plaintext << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}