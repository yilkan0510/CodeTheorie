// Plak dit in QuadgramScorer.h

#ifndef PLAYFAIR_QUADGRAMSCORER_H
#define PLAYFAIR_QUADGRAMSCORER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cmath>
#include <numeric>
#include <cctype>

class QuadgramScorer {
public:
    explicit QuadgramScorer(const std::string& quadgram_filepath) {
        std::ifstream file(quadgram_filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Kon quadgram-bestand niet openen: " + quadgram_filepath);
        }

        std::string quadgram;
        long long count;
        long long total_count = 0;
        std::map<std::string, long long> counts;

        while (file >> quadgram >> count) {
            counts[quadgram] = count;
            total_count += count;
        }

        for (auto const& [key, val] : counts) {
            log_probabilities[key] = log10(static_cast<double>(val) / total_count);
        }

        floor_prob = log10(0.01 / total_count);
    }

    // --- METHODE 1: TOLERANT (voor find_transposition) ---
    // Negeert quadgrams met cijfers, bestraft ze niet.
    double score_tolerant(const std::string& text) const {
        double total_score = 0.0;
        if (text.length() < 4) {
            return floor_prob * 4.0;
        }

        for (size_t i = 0; i + 3 < text.length(); ++i) {
            char c1 = text[i];
            char c2 = text[i+1];
            char c3 = text[i+2];
            char c4 = text[i+3];

            if (std::isalpha(static_cast<unsigned char>(c1)) &&
                std::isalpha(static_cast<unsigned char>(c2)) &&
                std::isalpha(static_cast<unsigned char>(c3)) &&
                std::isalpha(static_cast<unsigned char>(c4))) {
                
                std::string quad;
                quad += std::toupper(static_cast<unsigned char>(c1));
                quad += std::toupper(static_cast<unsigned char>(c2));
                quad += std::toupper(static_cast<unsigned char>(c3));
                quad += std::toupper(static_cast<unsigned char>(c4));

                auto it = log_probabilities.find(quad);
                total_score += (it != log_probabilities.end()) ? it->second : floor_prob;
            }
        }
        return total_score;
    }

    // --- METHODE 2: STRIKT (voor solve_square) ---
    // Bestraft quadgrams met cijfers.
    double score_strict(const std::string& text) const {
        std::string clean;
        clean.reserve(text.size());
        for (char c : text) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                clean += std::toupper(static_cast<unsigned char>(c));
            }
        }

        if (clean.size() < 4) return floor_prob * 4.0;

        double total_score = 0.0;
        for (size_t i = 0; i + 3 < clean.length(); ++i) {
            std::string quad = clean.substr(i, 4);
            auto it = log_probabilities.find(quad);
            total_score += (it != log_probabilities.end()) ? it->second : floor_prob;
        }
        return total_score;
    }

private:
    std::map<std::string, double> log_probabilities;
    double floor_prob;
};

#endif // PLAYFAIR_QUADGRAMSCORER_H