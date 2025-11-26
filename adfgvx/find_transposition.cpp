// find_transposition.cpp - FINALE VERSIE
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
#include <thread>
#include <mutex>
#include <atomic>

#include "QuadgramScorer.h"
#include "ADFGVX.h"

// Struct om het resultaat van een *enkele* permutatie-test op te slaan
struct PermutationResult {
    double score;
    std::vector<int> key;

    // Nodig om te kunnen sorteren
    bool operator<(const PermutationResult& other) const {
        return score < other.score;
    }
};

// Gedeelde data voor de threads
std::mutex cout_mutex;
std::atomic<int> permutations_processed(0);
std::atomic<double> best_overall_score(-999999.0);

// Functie om morse code te converteren
std::string convertMorseToADFGVX(const std::string& morse_code) {
    const std::map<std::string, char> morse_map = {
        {".-",   'A'}, {"-..",  'D'}, {"..-.", 'F'},
        {"--.",  'G'}, {"...-", 'V'}, {"-..-", 'X'}
    };

    std::string adfgvx_text;
    std::string current_morse_char;

    for (char c : morse_code) {
        if (c == '.' || c == '-') {
            current_morse_char += c;
        } else if (c == '/') {
            if (!current_morse_char.empty()) {
                if (morse_map.count(current_morse_char)) {
                    adfgvx_text += morse_map.at(current_morse_char);
                }
                current_morse_char.clear();
            }
        }
        // Negeer alle andere karakters (spaties, newlines, etc.)
    }

    // Vergeet het laatste karakter niet als het bestand niet eindigt op een '/'
    if (!current_morse_char.empty()) {
        if (morse_map.count(current_morse_char)) {
            adfgvx_text += morse_map.at(current_morse_char);
        }
    }

    return adfgvx_text;
}

// Functie om bestand te laden
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

// DE FUNCTIE DIE ELKE THREAD UITVOERT
void worker_function(int thread_id, const std::string* ciphertext, const QuadgramScorer* scorer,
                     const std::vector<std::vector<int>>* all_permutations, size_t start_index, size_t end_index,
                     std::vector<PermutationResult>* thread_results) {

    ADFGVX cipher;
    std::mt19937  rng(std::random_device{}() + thread_id);
    const int ITERATIONS_PER_PERMUTATION = 20000;

    // --- NIEUW: Variabelen om het lokale record van deze thread bij te houden ---
    double best_score_in_thread = -999999.0;
    std::vector<int> best_key_in_thread;
    // --- EINDE NIEUW ---

    for (size_t i = start_index; i < end_index; ++i) {
        const auto& current_transpo_perm = (*all_permutations)[i];

        std::string parent_square_key = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::shuffle(parent_square_key.begin(), parent_square_key.end(), rng);

        cipher.setKeys(parent_square_key, current_transpo_perm);
        std::string parent_plain = cipher.decrypt(*ciphertext);
        double parent_score = parent_plain.empty() ? -1e9 : scorer->score_tolerant(parent_plain);
        double best_score_for_this_perm = parent_score;

        double temperature = 20.0;
        double cooling_rate = 0.995;
        int stagnation_counter = 0;
        const int stagnation_limit = 2000;

        for(int j = 0; j < ITERATIONS_PER_PERMUTATION; ++j) {
            std::string child_square_key = parent_square_key;

            int a = std::uniform_int_distribution<int>(0, 35)(rng);
            int b = std::uniform_int_distribution<int>(0, 35)(rng);
            std::swap(child_square_key[a], child_square_key[b]);

            cipher.setKeys(child_square_key, current_transpo_perm);
            std::string child_plain = cipher.decrypt(*ciphertext);
            if (child_plain.empty()) continue;

            double child_score = scorer->score_tolerant(child_plain);
            if (child_score > parent_score || exp((child_score - parent_score) / temperature) > std::uniform_real_distribution<>(0.0, 1.0)(rng)) {
                parent_score = child_score; parent_square_key = child_square_key;
            }

            if (parent_score > best_score_for_this_perm) {
                best_score_for_this_perm = parent_score;
                stagnation_counter = 0;
            } else {
                stagnation_counter++;
            }
            temperature *= cooling_rate;
            if(stagnation_counter > stagnation_limit){
                temperature = 15.0;
                stagnation_counter = 0;
            }
        }

        thread_results->push_back({best_score_for_this_perm, current_transpo_perm});

        // --- NIEUW: Check en print voor lokaal record van de thread ---
        if (best_score_for_this_perm > best_score_in_thread) {
            best_score_in_thread = best_score_for_this_perm;
            best_key_in_thread = current_transpo_perm;

            // Gebruik de mutex om de output netjes te houden
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "[Thread " << thread_id << "] Nieuw LOKAAL record! Score: " << best_score_in_thread << " met sleutel: ";
            for(int k : best_key_in_thread) std::cout << k << " ";
            std::cout << std::endl;
        }
        // --- EINDE NIEUW ---

        // --- DUBBELE FEEDBACK LOGICA (voor globaal record) ---
        double current_best = best_overall_score;
        if (best_score_for_this_perm > current_best) {
            if (best_overall_score.compare_exchange_strong(current_best, best_score_for_this_perm)) {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << ">>> [Thread " << thread_id << "] NIEUW ALGEMEEN RECORD! Score: " << best_score_for_this_perm << " met sleutel: ";
                for(int k : current_transpo_perm) std::cout << k << " ";
                std::cout << " <<<" << std::endl;
            }
        }

        permutations_processed++;
        if (permutations_processed % 100 == 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Progress: " << permutations_processed << "/" << all_permutations->size() << "... (Huidig record: " << best_overall_score << ")" << std::endl;
        }
    }
}

int main() {
    try {
        // Start vanuit de map adfgvx/ zodat basePath klopt.
        std::filesystem::path basePath = "../";
        std::string ciphertext_path = (basePath / "adfgvx" / "03-OPGAVE-adfgvx.txt").string();
        std::string quadgrams_path = (basePath / "data" / "spaceless_english_quadgrams.txt").string();

        std::cout << "Laden van quadgrams van: " << quadgrams_path << std::endl;
        QuadgramScorer scorer(quadgrams_path);
        std::cout << "Laden van morse code van: " << ciphertext_path << std::endl;
        std::string morse_ciphertext = loadFileContent(ciphertext_path);
        std::string ciphertext = convertMorseToADFGVX(morse_ciphertext);
        if (ciphertext.length() % 2 != 0) { ciphertext.pop_back(); }
        std::cout << "Morse code omgezet naar ADFGVX ciphertext (" << ciphertext.length() << " tekens).\n" << std::endl;

        std::cout << std::fixed << std::setprecision(2);

        std::cout << "--- FASE 1: Starten van PARALLELLE, GRONDIGE brute-force aanval op transpositie-sleutel ---" << std::endl;

unsigned int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) { // Fallback voor als hardware_concurrency() 0 retourneert
            num_threads = 1;
        }

        std::vector<std::vector<int>> all_permutations;
        std::vector<int> p = {0, 1, 2, 3, 4, 5, 6};
        do {
            all_permutations.push_back(p);
        } while (std::next_permutation(p.begin(), p.end()));

        size_t total_perms = all_permutations.size();
        std::cout << total_perms << " permutaties gegenereerd om te testen." << std::endl;

        // Bepaal het daadwerkelijke aantal te gebruiken threads (veiligheidscheck)
        unsigned int threads_to_use = std::min<unsigned int>(num_threads, static_cast<unsigned int>(total_perms));
        std::cout << "Detecteerde " << num_threads << " threads, gebruiken er " << threads_to_use << "." << std::endl;

        std::vector<std::thread> threads;
        std::vector<std::vector<PermutationResult>> results_per_thread(threads_to_use);
        size_t chunk_size = total_perms / threads_to_use;
        std::cout << "Elke thread verwerkt circa " << chunk_size << " permutaties." << std::endl;

        for (unsigned int i = 0; i < threads_to_use; ++i) {
            size_t start = i * chunk_size;
            // Gebruik 'threads_to_use' voor de check van de laatste thread
            size_t end = (i == threads_to_use - 1) ? total_perms : start + chunk_size;

            threads.emplace_back(worker_function, i, &ciphertext, &scorer, &all_permutations, start, end, &results_per_thread[i]);
        }

        for (auto& t : threads) {
            t.join();
        }

        // Verzamel alle resultaten in één grote lijst
        std::vector<PermutationResult> all_results;
        for(const auto& thread_res : results_per_thread) {
            all_results.insert(all_results.end(), thread_res.begin(), thread_res.end());
        }

        // Sorteer de lijst van beste naar slechtste score
        std::sort(all_results.rbegin(), all_results.rend());

        std::cout << "\n--- BRUTE-FORCE VOLTOOID ---" << std::endl;
        std::cout << "Top 10 beste transpositie-sleutels gevonden:\n" << std::endl;

        for(int i = 0; i < std::min((size_t)10, all_results.size()); ++i) {
            std::cout << " #" << i + 1 << ": Score = " << all_results[i].score << " | Sleutel: ";
            for(int k : all_results[i].key) {
                std::cout << k << " ";
            }
            std::cout << std::endl;
        }

        // Sla de top 10 op in het bestand
        std::ofstream key_file("best_transpo_key.txt");
        if (key_file.is_open()) {
            for(int i = 0; i < std::min((size_t)10, all_results.size()); ++i) {
                for (size_t j = 0; j < all_results[i].key.size(); ++j) {
                    key_file << all_results[i].key[j] << (j == all_results[i].key.size() - 1 ? "" : " ");
                }
                key_file << std::endl;
            }
            key_file.close();
            std::cout << "\nTop 10 beste sleutels zijn opgeslagen in best_transpo_key.txt" << std::endl;
            std::cout << "De eerste (beste) sleutel in dit bestand zal worden gebruikt door solve_square." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
