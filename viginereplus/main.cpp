// filename: vigenere_column_attack.cpp
// Compile: g++ -std=c++17 vigenere_column_attack.cpp -O2 -o attack
// Run: ./attack

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <numeric>
#include <random>
#include <chrono>
#include <cctype>
#include <iomanip>

using namespace std;

// ---------- Utility: lees bestand ----------
string readFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) throw runtime_error("Kon bestand niet openen: " + filepath);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ---------- Preprocessing: alleen letters, uppercase ----------
string preprocess(const string& s) {
    string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        if (isalpha(ch)) {
            out.push_back(static_cast<char>(toupper(ch)));
        }
    }
    return out;
}

// ---------- Bigram scoring (log-probabilities). Hoger is beter (minder negatief) ----------
double calculateBigramScore(const string& text) {
    static const map<string, double> log_probabilities = {
        {"TH", -2.71}, {"HE", -2.83}, {"IN", -3.20}, {"ER", -3.22}, {"AN", -3.32},
        {"RE", -3.42}, {"ES", -3.53}, {"ON", -3.55}, {"ST", -3.67}, {"NT", -3.72},
        {"EN", -3.73}, {"AT", -3.76}, {"ED", -3.85}, {"ND", -3.87}, {"TO", -3.92},
        {"OR", -3.95}, {"EA", -4.05}, {"IS", -4.18}, {"HI", -4.20}, {"AR", -4.32},
        {"AS", -4.33}, {"DE", -4.47}, {"RT", -4.52}, {"VE", -4.63}, {"TE", -4.85}
    };
    static const double floor_score = -6.0; // zachtere straf voor onbekende bigrams

    double total_score = 0.0;
    if (text.size() < 2) return floor_score * static_cast<double>(text.size());
    for (size_t i = 0; i + 1 < text.length(); ++i) {
        string bigram;
        bigram.push_back(text[i]);
        bigram.push_back(text[i+1]);
        auto it = log_probabilities.find(bigram);
        if (it != log_probabilities.end()) total_score += it->second;
        else total_score += floor_score;
    }
    return total_score;
}

// ---------- Splits ciphertext in k kolommen, eerste (rem) kolommen krijgen +1 teken ----------
vector<string> splitIntoColumns(const string& text, int k) {
    int N = static_cast<int>(text.size());
    int base = N / k;
    int rem = N % k; // first 'rem' columns have base+1
    vector<string> cols(k);
    int pos = 0;
    for (int c = 0; c < k; ++c) {
        int len = base + (c < rem ? 1 : 0);
        if (len > 0) {
            cols[c] = text.substr(pos, len);
            pos += len;
        } else {
            cols[c] = "";
        }
    }
    return cols;
}

// ---------- Bouw row-major tekst uit kolommen volgens 'order' ----------
string buildFromCols(const vector<string>& cols, const vector<int>& order) {
    int k = static_cast<int>(cols.size());
    int maxlen = 0;
    for (const auto& s : cols) if ((int)s.size() > maxlen) maxlen = (int)s.size();
    string out;
    out.reserve(static_cast<size_t>(maxlen) * k);
    for (int r = 0; r < maxlen; ++r) {
        for (int j = 0; j < k; ++j) {
            const string& col = cols[order[j]];
            if (r < (int)col.size()) out.push_back(col[r]);
        }
    }
    return out;
}

// ---------- Brute-force (alle permutaties) voor kleine k ----------
pair<vector<int>, double> bruteForcePermutations(const vector<string>& cols) {
    int k = static_cast<int>(cols.size());
    vector<int> indices(k);
    iota(indices.begin(), indices.end(), 0);
    double best_score = -1e300;
    vector<int> best_order;
    long long cnt = 0;
    // ensure starting from sorted order for next_permutation
    sort(indices.begin(), indices.end());
    do {
        string t = buildFromCols(cols, indices);
        double s = calculateBigramScore(t);
        if (s > best_score) {
            best_score = s;
            best_order = indices;
        }
        ++cnt;
    } while (next_permutation(indices.begin(), indices.end()));
    // optional: cout << "Brute forced " << cnt << " permutations\n";
    return {best_order, best_score};
}

// ---------- Hill-climbing single run: probeer alle pairwise swaps (lokale zoek) ----------
pair<vector<int>, double> hillClimbOnce(const vector<string>& cols, int tries_no_improve = 200) {
    int k = static_cast<int>(cols.size());
    vector<int> order(k);
    iota(order.begin(), order.end(), 0);
    static std::mt19937_64 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
    shuffle(order.begin(), order.end(), rng);

    string current_text = buildFromCols(cols, order);
    double current_score = calculateBigramScore(current_text);

    bool improved = true;
    int stagnant = 0;
    while (improved && stagnant < tries_no_improve) {
        improved = false;
        // loop alle paren; bij een verbetering houden we de swap
        for (int i = 0; i < k - 1; ++i) {
            for (int j = i + 1; j < k; ++j) {
                swap(order[i], order[j]);
                string t = buildFromCols(cols, order);
                double s = calculateBigramScore(t);
                if (s > current_score) {
                    current_score = s;
                    current_text = std::move(t);
                    improved = true;
                    stagnant = 0;
                    // blijf zoeken vanaf deze nieuwe configuratie
                } else {
                    swap(order[i], order[j]); // undo
                }
            }
        }
        if (!improved) stagnant++;
    }
    return {order, current_score};
}

// ---------- Meerdere restarts van hill-climb om lokale maxima te vermijden ----------
pair<vector<int>, double> hillClimbWithRestarts(const vector<string>& cols, int restarts = 50) {
    vector<int> best_order;
    double best_score = -1e300;
    for (int r = 0; r < restarts; ++r) {
        auto [ord, score] = hillClimbOnce(cols, 200);
        if (score > best_score) {
            best_score = score;
            best_order = ord;
        }
    }
    return {best_order, best_score};
}

// ---------- Test meerdere sleutel-lengtes en rapporteer top resultaten ----------
void tryKeyLengths(const string& text, int min_k, int max_k) {
    struct Result { int k; double score; string sample; vector<int> order; };
    vector<Result> results;
    for (int k = min_k; k <= max_k; ++k) {
        cout << "Probeer k = " << k << " ... " << flush;
        auto cols = splitIntoColumns(text, k);
        pair<vector<int>, double> best;
        if (k <= 10) {
            best = bruteForcePermutations(cols);
        } else {
            best = hillClimbWithRestarts(cols, 60);
        }
        string best_text = buildFromCols(cols, best.first);
        cout << "done. score=" << fixed << setprecision(6) << best.second << " sample='" 
             << best_text.substr(0, min<size_t>(best_text.size(), 60)) << "'\n";
        results.push_back({k, best.second, best_text.substr(0, 200), best.first});
    }

    sort(results.begin(), results.end(), [](const Result& a, const Result& b){ return a.score > b.score; });

    cout << "\n--- Top resultaten ---\n";
    for (size_t i = 0; i < results.size(); ++i) {
        cout << i+1 << ") k=" << results[i].k << "  score=" << fixed << setprecision(6) << results[i].score << "\n";
        cout << "   sample: " << results[i].sample << "...\n";
        cout << "   order: ";
        for (int v : results[i].order) cout << v << " ";
        cout << "\n";
    }

    if (!results.empty()) {
        // Schrijf de volledige tekst voor de beste gevonden lengte naar bestand
        int best_k = results[0].k;
        auto best_cols = splitIntoColumns(text, best_k);
        string best_full = buildFromCols(best_cols, results[0].order);
        ofstream out("../../viginereplus/output_best_text.txt");
        if (out) {
            out << best_full;
            cout << "\nBeste volledige tekst (k=" << best_k << ") weggeschreven naar ../../viginereplus/output_best_text.txt\n";
        } else {
            cout << "\nKon output file niet schrijven\n";
        }
    }
}

// ---------- MAIN ----------
int main() {
    try {
        string raw = readFile("../../viginereplus/01-OPGAVE-viginereplus.txt");
        cout << "Bestand ingelezen: " << raw.size() << " tekens (voor preprocessing)\n";

        string text = preprocess(raw);
        cout << "Na preprocessing: " << text.size() << " letters (A-Z)\n";

        if (text.size() < 2) {
            cerr << "Te weinig letters om te analyseren.\n";
            return 1;
        }

        // Pas hier het bereik aan dat je wil testen
        int min_k = 2;
        int max_k = 14;
        cout << "Test sleutel-lengtes van " << min_k << " tot " << max_k << endl;

        tryKeyLengths(text, min_k, max_k);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
