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
#include <iomanip> // Toegevoegd voor betere output

#include "QuadgramScorer.h"
#include "ADFGVX.h"

// Morse code -> ADFGVX letters (ongewijzigd)
std::string convertMorseToADFGVX(const std::string& morse_code) {
    const std::map<std::string, char> morse_map = {
        {".-",   'A'}, {"-..",  'D'}, {"..-.", 'F'},
        {"--.",  'G'}, {"...-", 'V'}, {"-..-", 'X'}
    };
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

// Lees bestand (ongewijzigd)
std::string loadFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Kon bestand niet openen: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();
    text.erase(std::remove_if(text.begin(), text.end(), ::isspace), text.end());
    return text;
}

int main() {
    try {
        // --- CONFIGURATIE ---
        std::filesystem::path basePath = "../../";
        std::string ciphertext_path = (basePath / "adfgvx" / "03-OPGAVE-adfgvx.txt").string();
        std::string quadgrams_path = (basePath / "data" / "english_quadgrams.txt").string();

        const int MIN_KEY_LEN = 7;
        const int MAX_KEY_LEN = 7;
        const int MAX_ITERATIONS = 20000000;

        // --- INITIALISATIE ---
        std::cout << "Laden van quadgrams van: " << quadgrams_path << std::endl;
        QuadgramScorer scorer(quadgrams_path);

        std::cout << "Laden van morse code van: " << ciphertext_path << std::endl;
        std::string morse_ciphertext = loadFileContent(ciphertext_path);
        std::string ciphertext = convertMorseToADFGVX(morse_ciphertext);

        if (ciphertext.length() % 2 != 0) {
            ciphertext.pop_back();
        }

        std::cout << "Morse code omgezet naar ADFGVX ciphertext (" << ciphertext.length() << " tekens)." << std::endl;
        std::cout << "Preview: " << ciphertext.substr(0, 60) << "...\n" << std::endl;

        ADFGVX cipher;
        std::mt19937 rng(std::random_device{}());

        double overall_best_score = -999999.0;
        std::string best_plaintext;
        std::string best_square_key;
        std::vector<int> best_transpo_key;

        for (int key_len = MIN_KEY_LEN; key_len <= MAX_KEY_LEN; ++key_len) {
            std::cout << "\n=======================================================\n";
            std::cout << "   Testen van sleutellengte: " << key_len << "\n";
            std::cout << "=======================================================\n";

            // --- STARTEN MET JOUW BESTE GEVONDEN SLEUTEL ---
            std::string parent_square_key = "S57089KMA4JFHWZ1GVUDLCQEYROPX2T3NIB6";
            std::vector<int> parent_transpo_order = {5, 1, 2, 6, 0, 4, 3};

            double best_score_for_len = -999999.0;
            double parent_score;

            cipher.setKeys(parent_square_key, parent_transpo_order);
            std::string initial_plain = cipher.decrypt(ciphertext);
            parent_score = initial_plain.empty() ? -1e9 : scorer.score(initial_plain);

            if (parent_score > best_score_for_len) {
                best_score_for_len = parent_score;
                if (best_score_for_len > overall_best_score) {
                    overall_best_score = best_score_for_len;
                    best_square_key = parent_square_key;
                    best_transpo_key = parent_transpo_order;
                    best_plaintext = initial_plain;
                }
            }

            double temperature = 40.0;
            double cooling_rate = 0.99999;

            int stagnation_counter = 0;
            const int stagnation_limit = 500000;

            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Startpunt score: " << parent_score << std::endl;


            for (int i = 0; i < MAX_ITERATIONS; ++i) {
                if (i > 0 && i % 50000 == 0) {
                    std::cout << "Iter: " << i / 1000 << "k | "
                              << "Huidige Score: " << parent_score << " | "
                              << "Beste Score: " << overall_best_score << " | "
                              << "Temp: " << temperature
                              << std::endl;
                }

                std::string child_square_key = parent_square_key;
                std::vector<int> child_transpo_order = parent_transpo_order;

                if (std::uniform_real_distribution<>(0.0, 1.0)(rng) < 0.5) {
                    int a = std::uniform_int_distribution<int>(0, key_len - 1)(rng);
                    int b = std::uniform_int_distribution<int>(0, key_len - 1)(rng);
                    std::swap(child_transpo_order[a], child_transpo_order[b]);
                } else {
                    if (std::uniform_real_distribution<>(0.0, 1.0)(rng) < 0.95) {
                      int pos_a = -1, pos_b = -1;
                      while (pos_a == -1 || !std::isalpha(child_square_key[pos_a])) { pos_a = std::uniform_int_distribution<int>(0, 35)(rng); }
                      while (pos_b == -1 || pos_b == pos_a || !std::isalpha(child_square_key[pos_b])) { pos_b = std::uniform_int_distribution<int>(0, 35)(rng); }
                      std::swap(child_square_key[pos_a], child_square_key[pos_b]);
                    } else {
                      int a = std::uniform_int_distribution<int>(0, 35)(rng);
                      int b = std::uniform_int_distribution<int>(0, 35)(rng);
                      std::swap(child_square_key[a], child_square_key[b]);
                    }
                }

                cipher.setKeys(child_square_key, child_transpo_order);
                std::string child_plain = cipher.decrypt(ciphertext);
                if (child_plain.empty()) continue;

                double child_score = scorer.score(child_plain);
                double delta = child_score - parent_score;

                if (delta > 0 || exp(delta / temperature) > std::uniform_real_distribution<>(0.0, 1.0)(rng)) {
                    parent_score = child_score;
                    parent_square_key = child_square_key;
                    parent_transpo_order = child_transpo_order;
                }

                if (parent_score > best_score_for_len) {
                    best_score_for_len = parent_score;
                    stagnation_counter = 0;
                    if (best_score_for_len > overall_best_score) {
                        overall_best_score = best_score_for_len;
                        best_square_key = parent_square_key;
                        best_transpo_key = parent_transpo_order;
                        best_plaintext = cipher.decrypt(ciphertext);

                        std::cout << "\n>>> NIEUW RECORD <<<"
                                  << " Score: " << overall_best_score
                                  << " (Iter " << i << ")" << std::endl;
                        std::cout << "    Tekst: " << best_plaintext.substr(0, 80) << "...\n" << std::endl;
                    }
                } else {
                    stagnation_counter++;
                }

                temperature *= cooling_rate;

                if (stagnation_counter >= stagnation_limit) {
                    std::cout << "\n--- STAGNATIE! HERVERHITTEN ---\n" << std::endl;
                    temperature = 30.0;
                    stagnation_counter = 0;
                }
            }
        }

        // --- Finale output naar console ---
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