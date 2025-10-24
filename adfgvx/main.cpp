// adfgvx/main.cpp (Versie 2 - met duidelijkere output)
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

#include "QuadgramScorer.h"
#include "ADFGVX.h"

// Functie om Morse code naar ADFGVX-tekens te converteren
std::string convertMorseToADFGVX(const std::string& morse_code) {
    const std::map<std::string, char> morse_map = {
        {".-",   'A'}, {"-..",  'D'}, {"..-.", 'F'},
        {"--.",  'G'}, {"...-", 'V'}, {"-..-", 'X'}
    };

    std::string adfgvx_text;
    std::stringstream ss(morse_code);
    std::string single_morse_char;

    while(std::getline(ss, single_morse_char, '/')) {
        if (morse_map.count(single_morse_char)) {
            adfgvx_text += morse_map.at(single_morse_char);
        }
    }
    return adfgvx_text;
}

// Functie om het ciphertext-bestand te lezen
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

        const int MIN_KEY_LEN = 7 ;
        const int MAX_KEY_LEN = 7;
        const int MAX_ITERATIONS = 2000000; // Verhoogd voor een diepere zoektocht
//        const std::string ALPHABET_GRID = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        const std::string ALPHABET_GRID = "O20V1PI67CKEDT3ZXUANMBFGRQ8594SJWHLY";

        // --- INITIALISATIE ---
        std::cout << "Laden van quadgrams van: " << quadgrams_path << std::endl;
        QuadgramScorer scorer(quadgrams_path);

        std::cout << "Laden van morse code van: " << ciphertext_path << std::endl;
        std::string morse_ciphertext = loadFileContent(ciphertext_path);

        std::string ciphertext = convertMorseToADFGVX(morse_ciphertext);
        std::cout << "Morse code omgezet naar ADFGVX ciphertext (" << ciphertext.length() << " tekens)." << std::endl;
        std::cout << "Preview: " << ciphertext.substr(0, 60) << "..." << std::endl;

        ADFGVX cipher;
        std::mt19937 rng(std::random_device{}());

        double overall_best_score = -999999.0;
        std::string best_plaintext;
        std::string best_square_key;
        std::vector<int> best_transpo_key;

        // --- HOOFDLUS: ITEREER OVER MOGELIJKE SLEUTELLENGTES ---
        for (int key_len = MIN_KEY_LEN; key_len <= MAX_KEY_LEN; ++key_len) {
            std::cout << "\n\n=======================================================" << std::endl;
            std::cout << "   Testen van sleutellengte: " << key_len << std::endl;
            std::cout << "=======================================================\n" << std::endl;

            std::string parent_square_key = ALPHABET_GRID;
            std::shuffle(parent_square_key.begin(), parent_square_key.end(), rng);

            std::vector<int> parent_transpo_order(key_len);
            std::iota(parent_transpo_order.begin(), parent_transpo_order.end(), 0);
            std::shuffle(parent_transpo_order.begin(), parent_transpo_order.end(), rng);

            cipher.setKeys(parent_square_key, parent_transpo_order);
            double parent_score = scorer.score(cipher.decrypt(ciphertext));
            double best_score_for_len = parent_score;

            double temperature = 20.0;
            double cooling_rate = 0.99998; // Iets langzamer afkoelen kan helpen

            for (int i = 0; i < MAX_ITERATIONS; ++i) {
                // --- NIEUW: Periodieke voortgangsupdate ---
                if (i > 0 && i % 50000 == 0) {
                    std::cout << "  ... Progress voor lengte " << key_len << ": " << i << " / " << MAX_ITERATIONS << " iteraties. Huidige score: " << parent_score << std::endl;
                }

                std::string child_square_key = parent_square_key;
                std::vector<int> child_transpo_order = parent_transpo_order;

                if (std::uniform_real_distribution<>(0.0, 1.0)(rng) < 0.5) {
                    int a = std::uniform_int_distribution<int>(0, key_len - 1)(rng);
                    int b = std::uniform_int_distribution<int>(0, key_len - 1)(rng);
                    std::swap(child_transpo_order[a], child_transpo_order[b]);
                } else {
                    int a = std::uniform_int_distribution<int>(0, 35)(rng);
                    int b = std::uniform_int_distribution<int>(0, 35)(rng);
                    std::swap(child_square_key[a], child_square_key[b]);
                }

                cipher.setKeys(child_square_key, child_transpo_order);
                double child_score = scorer.score(cipher.decrypt(ciphertext));

                double delta = child_score - parent_score;
                if (delta > 0 || exp(delta / temperature) > std::uniform_real_distribution<>(0.0, 1.0)(rng)) {
                    parent_score = child_score;
                    parent_square_key = child_square_key;
                    parent_transpo_order = child_transpo_order;
                }

                if (parent_score > best_score_for_len) {
                    best_score_for_len = parent_score;
                    if (best_score_for_len > overall_best_score) {
                        overall_best_score = best_score_for_len;
                        best_square_key = parent_square_key;
                        best_transpo_key = parent_transpo_order;
                        best_plaintext = cipher.decrypt(ciphertext);

                        // --- NIEUW: Duidelijkere output bij een nieuw record ---
                        std::cout << "\n-------------------------------------------------------\n";
                        std::cout << ">>> Nieuw Globaal Maximum Gevonden! <<<\n";
                        std::cout << "    Sleutellengte: " << key_len << "\n";
                        std::cout << "    Iteratie:      " << i << " / " << MAX_ITERATIONS << "\n";
                        std::cout << "    Score:         " << overall_best_score << "\n";
                        std::cout << "    Temperatuur:   " << temperature << "\n";
                        std::cout << "    Beste Tekst:   " << best_plaintext.substr(0, 100) << "...\n";
                        std::cout << "-------------------------------------------------------\n" << std::endl;
                    }
                }
                temperature *= cooling_rate;
            }
        }

        // --- RESULTAAT ---
        std::cout << "\n\n--- Aanval voltooid ---" << std::endl;
        std::cout << "Beste gevonden score: " << overall_best_score << std::endl;
        std::cout << "Beste Polybius Square sleutel: " << best_square_key << std::endl;
        std::cout << "Beste Transpositie sleutel (lengte " << best_transpo_key.size() << ", volgorde): ";
        for(int i : best_transpo_key) std::cout << i << " ";
        std::cout << std::endl;
        std::cout << "\nOntsleutelde tekst:" << std::endl;
        std::cout << best_plaintext << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}