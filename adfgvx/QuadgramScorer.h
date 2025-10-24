// playfair/QuadgramScorer.h
#ifndef PLAYFAIR_QUADGRAMSCORER_H
#define PLAYFAIR_QUADGRAMSCORER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cmath>
#include <numeric>

class QuadgramScorer {
public:
    // Constructor laadt de frequenties uit een bestand
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

        // Bereken log-waarschijnlijkheden voor numerieke stabiliteit
        for (auto const& [key, val] : counts) {
            log_probabilities[key] = log10(static_cast<double>(val) / total_count);
        }

        // Een "bodem" score voor quadgrams die niet in onze lijst staan
        floor_prob = log10(0.01 / total_count);
    }

    // Berekent de fitness-score van een gegeven tekst
    double score(const std::string& text) const {
        double total_score = 0.0;
        for (size_t i = 0; i < text.length() - 3; ++i) {
            std::string quad = text.substr(i, 4);
            auto it = log_probabilities.find(quad);
            if (it != log_probabilities.end()) {
                total_score += it->second;
            } else {
                total_score += floor_prob;
            }
        }
        return total_score;
    }

private:
    std::map<std::string, double> log_probabilities;
    double floor_prob;
};

#endif //PLAYFAIR_QUADGRAMSCORER_H