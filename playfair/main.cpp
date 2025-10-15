// playfair/main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <filesystem> // C++17 voor paden
#include <cmath>      // voor exp()

#include "QuadgramScorer.h"
#include "Playfair.h"

// Functie om de ciphertext uit het bestand te lezen en voor te bereiden (ongewijzigd)
std::string loadAndPrepareCiphertext(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Kon ciphertext-bestand niet openen: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    std::string cleaned_text;
    for (char c : text) {
        if (isalpha(c)) {
            cleaned_text += static_cast<char>(toupper(c));
        }
    }

    std::replace(cleaned_text.begin(), cleaned_text.end(), 'J', 'I');

    if (cleaned_text.size() % 2 != 0) {
        cleaned_text.pop_back();
    }

    return cleaned_text;
}

int main() {
    try {
        // --- CONFIGURATIE ---
        std::filesystem::path basePath = "../../";
        std::string ciphertext_path = (basePath / "playfair" / "02-OPGAVE-playfair.txt").string();
        std::string quadgrams_path = (basePath / "data" / "spanish_quadgrams.txt").string();
        std::string output_filepath = (basePath / "playfair" / "decrypted_solution.txt").string();

        const int MAX_ITERATIONS = 3000000;

        // --- INITIALISATIE ---
        std::cout << "Laden van quadgrams van: " << quadgrams_path << std::endl;
        QuadgramScorer scorer(quadgrams_path);

        std::cout << "Laden van ciphertext van: " << ciphertext_path << std::endl;
        std::string ciphertext = loadAndPrepareCiphertext(ciphertext_path);
        std::cout << "Ciphertext geladen (" << ciphertext.length() << " tekens)." << std::endl;

        Playfair cipher;
        std::mt19937 rng(std::random_device{}());

//        used in begin. we are starting from a better key now. no need to shuffle
//        std::string parent_key = "ABCDEFGHIKLMNOPQRSTUVWXYZ";
//        std::shuffle(parent_key.begin(), parent_key.end(), rng);
        std::string parent_key = "YTVWXIGABRQELCMHUZDFSKNOP";
        std::string best_key = parent_key;

        cipher.setKey(parent_key);
        double parent_score = scorer.score(cipher.decrypt(ciphertext));

        double best_score = parent_score;

        // --- SIMULATED ANNEALING LOOP ---
        std::cout << "\nStarten van de simulated annealing aanval..." << std::endl;

        double temperature = 50.0;
        double cooling_rate = 0.999995;
        int stagnation_limit = 20000;
        int stagnation_counter = 0;

        std::uniform_int_distribution<int> dist(0, 24);
        std::uniform_real_distribution<double> real_dist(0.0, 1.0);

        for (int i = 0; i < MAX_ITERATIONS; ++i) {
            std::string child_key = parent_key;
            int choice = rng() % 2;  // choose between two mutation types

            if (choice == 0) {
                // Type 1: swap two random letters
                int a = dist(rng), b = dist(rng);
                std::swap(child_key[a], child_key[b]);
            } else {
                // Type 2: reverse a random section (helps larger changes)
                int start = dist(rng), end = dist(rng);
                if (start > end) std::swap(start, end);
                std::reverse(child_key.begin() + start, child_key.begin() + end);
            }

            cipher.setKey(child_key);
            double child_score = scorer.score(cipher.decrypt(ciphertext));
            double delta = child_score - parent_score;

            if (delta > 0 || exp(delta / temperature) > real_dist(rng)) {
                parent_key = child_key;
                parent_score = child_score;
            }

            if (parent_score > best_score) {
                best_score = parent_score;
                best_key = parent_key;
                stagnation_counter = 0;

                std::cout << "Iteratie " << i << " | Beste Score: " << best_score << " | Temp: " << temperature << std::endl;
                std::cout << "Tekst: " << cipher.decrypt(ciphertext).substr(0, 100) << "..." << std::endl;
                std::cout << "Sleutel: " << best_key << "\n" << std::endl;
            } else {
                stagnation_counter++;
            }

            temperature *= cooling_rate;

            if (stagnation_counter >= stagnation_limit) {
                std::cout << "\n--- Stagnatie bereikt. Herstart bij iteratie " << i << " ---\n" << std::endl;

//        used in begin. we are starting from a better key now. no need to shuffle
//                std::shuffle(parent_key.begin(), parent_key.end(), rng);

            parent_key = best_key;
            int a = dist(rng), b = dist(rng);
            std::swap(parent_key[a], parent_key[b]);
            cipher.setKey(parent_key);
            parent_score = scorer.score(cipher.decrypt(ciphertext));
            stagnation_counter = 0;
            temperature = 20.0;
            }
        }

        // --- RESULTAAT ---
        std::cout << "\n--- Aanval voltooid ---" << std::endl;
        std::cout << "Beste gevonden sleutel: " << best_key << std::endl;
        std::cout << "Beste score: " << best_score << std::endl;

        cipher.setKey(best_key);
        std::string decrypted_text = cipher.decrypt(ciphertext);

        std::cout << "Ontsleutelde tekst:" << std::endl;
        std::cout << decrypted_text.substr(0, 500) << "..." << std::endl; // Toon een groter deel in de console

        // --- NIEUW: Schrijf het eindresultaat naar een bestand ---
        std::cout << "\nSchrijven van resultaat naar: " << output_filepath << std::endl;
        std::ofstream output_file(output_filepath);
        if (output_file.is_open()) {
            output_file << "Beste gevonden sleutel: " << best_key << std::endl;
            output_file << "Beste score: " << best_score << std::endl;
            output_file << "Aantal iteraties: " << MAX_ITERATIONS << std::endl;
            output_file << "------------------------------------\n\n";
            output_file << decrypted_text << std::endl;
            output_file.close();
            std::cout << "Resultaat succesvol opgeslagen." << std::endl;
        } else {
            std::cerr << "Fout: Kon het output-bestand niet openen: " << output_filepath << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}