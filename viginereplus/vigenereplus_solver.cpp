#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <numeric>
#include <map>
#include <cmath>
#include <iomanip>
#include <stdexcept>

using namespace std;

// Lees de ciphertext uit een bestand
string load_ciphertext(const string& path) {
    ifstream in(path);
    if (!in) {
        throw runtime_error("Kon ciphertext bestand niet openen: " + path);
    }
    return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
}

// Nederlandse letterfrequenties (A=0, B=1, ...)
const double DUTCH_FREQ[26] = {
    7.49, 1.58, 1.24, 5.93, 18.91, 0.81, 3.40, 2.38, 6.50, 1.46, 2.25, 3.57, 2.21,
    10.03, 6.06, 1.57, 0.01, 6.41, 3.73, 6.79, 1.99, 2.85, 1.52, 0.04, 0.03, 1.39
};


// 1. Transpositie ongedaan maken
string decrypt_columnar(const string& cipher, const vector<int>& key) {
    int len = cipher.length();
    int width = key.size();
    int height = (len + width - 1) / width; // ceil(len/width)
    int num_full_cols = len % width;
    if (num_full_cols == 0) num_full_cols = width;

    // Grid buffers voor elke kolom
    vector<string> grid(width);
    
    int current_pos = 0;
    // De 'key' bevat de volgorde waarin de kolommen zijn uitgelezen.
    // We itereren door de sleutel om de chunks uit de ciphertext te halen.
    for (int k : key) {
        // Een fysieke kolom 'k' is lang als k < num_full_cols, anders kort
        int col_len = (k < num_full_cols) ? height : height - 1;
        grid[k] = cipher.substr(current_pos, col_len);
        current_pos += col_len;
    }

    // Nu lezen we het rij voor rij uit
    string plain;
    plain.reserve(len);
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            if (r < grid[c].length()) {
                plain += grid[c][r];
            }
        }
    }
    return plain;
}

// Helper voor IC (Index of Coincidence)
double calculate_ic(const string& text) {
    if (text.length() <= 1) return 0.0;
    vector<int> counts(26, 0);
    for (char c : text) counts[c - 'A']++;
    
    double sum = 0;
    for (int c : counts) sum += c * (c - 1);
    
    return sum / (text.length() * (text.length() - 1));
}

// Score hoe "Vigenere-achtig" de tekst is.
// Als de transpositie goed is, moeten de periodieke slices een hoge IC hebben (NL ~0.077)
// Als de transpositie fout is, is het random (~0.038)
double score_vigenere_potential(const string& text) {
    double max_avg_ic = 0.0;
    
    // We checken mogelijke Vigenere sleutellengtes 2 t/m 10
    for (int key_len = 1; key_len <= 10; ++key_len) {
        double total_ic = 0.0;
        for (int i = 0; i < key_len; ++i) {
            string slice;
            for (int j = i; j < text.length(); j += key_len) {
                slice += text[j];
            }
            total_ic += calculate_ic(slice);
        }
        double avg_ic = total_ic / key_len;
        if (avg_ic > max_avg_ic) max_avg_ic = avg_ic;
    }
    return max_avg_ic;
}

// Vigenere kraker (frequentie analyse per kolom)
string solve_vigenere(const string& text) {
    string decrypted = text;
    string best_key = "";
    
    // Bepaal eerst de beste sleutellengte o.b.v. IC
    int best_len = 1;
    double best_ic = 0;
    for (int l = 1; l <= 10; ++l) {
         double total_ic = 0.0;
        for (int i = 0; i < l; ++i) {
            string slice;
            for (int j = i; j < text.length(); j += l) slice += text[j];
            total_ic += calculate_ic(slice);
        }
        double avg = total_ic / l;
        if (avg > best_ic) { best_ic = avg; best_len = l; }
    }

    // Nu per kolom de caesar shift bepalen (Chi-squared)
    for (int i = 0; i < best_len; ++i) {
        string slice;
        for (int j = i; j < text.length(); j += best_len) slice += text[j];
        
        int best_shift = 0;
        double min_chi = 1e9;
        
        for (int shift = 0; shift < 26; ++shift) {
            double chi = 0;
            vector<int> counts(26, 0);
            for (char c : slice) counts[(c - 'A' - shift + 26) % 26]++;
            
            for (int k = 0; k < 26; ++k) {
                double expected = DUTCH_FREQ[k] * slice.length() / 100.0;
                double diff = counts[k] - expected;
                chi += (diff * diff) / expected;
            }
            if (chi < min_chi) { min_chi = chi; best_shift = shift; }
        }
        best_key += (char)('A' + best_shift);
    }

    // Decrypt met gevonden sleutel
    for (int i = 0; i < text.length(); ++i) {
        int shift = best_key[i % best_len] - 'A';
        decrypted[i] = (text[i] - 'A' - shift + 26) % 26 + 'A';
    }
    
        return "\n=== KEY: " + best_key + " ===\n\n" + decrypted 
        + "\n\n=============================================";
}

int main() {
    string text;
    try {
        text = load_ciphertext("01-OPGAVE-viginereplus.txt");
    } catch (const exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }

    cout << "Start kraken (dit kan enkele minuten duren)..." << endl;

    // Probeer elke transpositie sleutel lengte
    for (int width = 2; width <= 10; ++width) {
        vector<int> p(width);
        iota(p.begin(), p.end(), 0); // Vul met 0, 1, 2...

        // Probeer elke permutatie
        do {
            string trans_candidate = decrypt_columnar(text, p);
            
            // Filter: Is dit een valide Vigenere tekst?
            // Nederlandse Vigenere heeft IC rond 0.077 in kolommen. Random is 0.038.
            // We zetten de drempel op 0.065 om ruis eruit te filteren.
            double score = score_vigenere_potential(trans_candidate);
            
            if (score > 0.077) { // Strenge filter
                cout << "\n[!] Mogelijke hit gevonden!" << endl;
                cout << "Transpositie Breedte: " << width << endl;
                cout << "Transpositie Permutatie: ";
                for(int k : p) cout << k << " ";
                cout << endl;
                cout << "IC Score: " << score << endl;
                
                // Probeer Vigenere op te lossen en print resultaat
                cout << solve_vigenere(trans_candidate) << endl;
            }

        } while (next_permutation(p.begin(), p.end()));
        
        cout << "Klaar met breedte " << width << "..." << endl;
    }

    return 0;
}
