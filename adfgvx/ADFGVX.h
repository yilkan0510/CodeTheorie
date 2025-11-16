#ifndef ADFGVX_H
#define ADFGVX_H

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <utility>
#include <numeric>
#include <algorithm>
#include <array>

class ADFGVX {
public:
    // Sets the two keys needed for decryption.
    // square_key: A 36-character string for the Polybius square.
    // transpo_order: A vector defining the order to read columns (e.g., {3, 1, 0, 2}).
    void setKeys(const std::string& square_key, const std::vector<int>& transpo_order) {
        if (square_key.length() != 36) {
            throw std::invalid_argument("Polybius square key must be 36 characters long.");
        }
        if (transpo_order.empty()) {
            throw std::invalid_argument("Transposition order cannot be empty.");
        }

        // Check that square_key has unique characters
        std::array<bool, 256> seen_chars{};
        for (char c : square_key) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (seen_chars[uc]) {
                throw std::invalid_argument("Polybius square key contains duplicate characters.");
            }
            seen_chars[uc] = true;
        }

        // Validate that transpo_order is a valid permutation of 0..n-1
        int n = transpo_order.size();
        std::vector<bool> seen(n, false);
        for (int v : transpo_order) {
            if (v < 0 || v >= n || seen[v]) {
                throw std::invalid_argument("Invalid transposition order: must be a permutation of 0..n-1.");
            }
            seen[v] = true;
        }

        this->transposition_order = transpo_order;

        // Build the 6x6 grid and the coordinate map for fast lookups
        const std::string adfgvx_chars = "ADFGVX";
        coords_to_char.clear();
        for (int i = 0; i < 36; ++i) {
            int row = i / 6;
            int col = i % 6;
            grid[row][col] = square_key[i];

            // Map the coordinate pair (e.g., "AD") to the character in the grid
            std::string coords_str;
            coords_str += adfgvx_chars[row];
            coords_str += adfgvx_chars[col];
            coords_to_char[coords_str] = square_key[i];
        }
    }

    // Decrypts the ciphertext using the currently set keys.
    std::string decrypt(const std::string& ciphertext) const {
        if (transposition_order.empty()) {
            throw std::runtime_error("Keys are not set.");
        }

        // First, undo the columnar transposition
        std::string intermediate_text = undoColumnarTransposition(ciphertext);
        if (intermediate_text.empty()) return "";

        // Second, undo the substitution from the Polybius square
        return performSubstitution(intermediate_text);
    }

private:
    std::vector<int> transposition_order;
    char grid[6][6]{};
    std::map<std::string, char> coords_to_char;

    // Reverses the columnar transposition stage.
std::string undoColumnarTransposition(const std::string& ciphertext) const {
    int num_cols = transposition_order.size();
    if (num_cols == 0) return "";

    int text_len = ciphertext.length();
    int base_col_len = text_len / num_cols;
    int long_cols = text_len % num_cols;

    // --- NIEUWE, EENVOUDIGERE LOGICA ---

    // Stap 1: Bepaal de lengte van elke kolom in de *gelezen* (gesorteerde) volgorde.
    std::vector<int> col_lengths(num_cols);
    for (int i = 0; i < num_cols; ++i) {
        int original_col_index = transposition_order[i];
        col_lengths[i] = base_col_len + (original_col_index < long_cols ? 1 : 0);
    }

    // Stap 2: Maak een tabel (vector van strings) en vul deze met de juiste stukken ciphertext.
    std::vector<std::string> columns(num_cols);
    int current_pos = 0;
    for (int i = 0; i < num_cols; ++i) {
        int original_col_index = transposition_order[i];
        int len = col_lengths[i];
        columns[original_col_index] = ciphertext.substr(current_pos, len);
        current_pos += len;
    }

    // --- EINDE NIEUWE LOGICA ---

    // Stap 3: Lees de tabel rij voor rij uit. Dit deel was al correct.
    std::string intermediate_text;
    intermediate_text.reserve(text_len);
    int max_rows = base_col_len + (long_cols > 0 ? 1 : 0);
    for (int row = 0; row < max_rows; ++row) {
        for (int col = 0; col < num_cols; ++col) {
            if (row < columns[col].length()) {
                intermediate_text += columns[col][row];
            }
        }
    }
    return intermediate_text;
}

    // Reverses the substitution stage (converts digraphs back to single characters).
    std::string performSubstitution(const std::string& intermediate_text) const {
        if (intermediate_text.length() % 2 != 0) {
            return ""; // invalid length
        }

        std::string plaintext;
        plaintext.reserve(intermediate_text.length() / 2);
        for (size_t i = 0; i < intermediate_text.length(); i += 2) {
            std::string digraph = intermediate_text.substr(i, 2);
            auto it = coords_to_char.find(digraph);
            if (it != coords_to_char.end()) {
                plaintext += it->second;
            } else {
                return ""; // invalid digraph
            }
        }
        return plaintext;
    }
};

#endif // ADFGVX_H
