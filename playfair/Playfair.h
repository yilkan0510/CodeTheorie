// playfair/Playfair.h
#ifndef PLAYFAIR_PLAYFAIR_H
#define PLAYFAIR_PLAYFAIR_H

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <utility> // for std::pair

class Playfair {
public:
    // Stelt de sleutel in en bouwt de interne 5x5 matrix
    void setKey(const std::string& key) {
        if (key.length() != 25) {
            throw std::invalid_argument("Sleutel moet 25 unieke karakters bevatten.");
        }
        key_string = key;
        coords.clear();
        for (int i = 0; i < 25; ++i) {
            int row = i / 5;
            int col = i % 5;
            grid[row][col] = key[i];
            coords[key[i]] = {row, col};
        }
    }

    // Ontsleutelt de ciphertext
    std::string decrypt(const std::string& ciphertext) const {
        std::string plaintext = "";
        for (size_t i = 0; i < ciphertext.length(); i += 2) {
            char c1 = ciphertext[i];
            char c2 = ciphertext[i + 1];

            auto pos1 = coords.at(c1);
            auto pos2 = coords.at(c2);

            int r1 = pos1.first, c1_coord = pos1.second;
            int r2 = pos2.first, c2_coord = pos2.second;

            if (r1 == r2) { // Zelfde rij
                plaintext += grid[r1][(c1_coord + 4) % 5];
                plaintext += grid[r2][(c2_coord + 4) % 5];
            } else if (c1_coord == c2_coord) { // Zelfde kolom
                plaintext += grid[(r1 + 4) % 5][c1_coord];
                plaintext += grid[(r2 + 4) % 5][c2_coord];
            } else { // Rechthoek
                plaintext += grid[r1][c2_coord];
                plaintext += grid[r2][c1_coord];
            }
        }
        return plaintext;
    }

private:
    std::string key_string;
    char grid[5][5]{};
    std::map<char, std::pair<int, int>> coords;
};


#endif //PLAYFAIR_PLAYFAIR_H