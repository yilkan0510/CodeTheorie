#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

string readFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) throw runtime_error("Kon bestand niet openen: " + filepath);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

vector<vector<char>> textToMatrix(const string& text, int num_cols) {
    int col_len = text.length() / num_cols;
    vector<vector<char>> matrix(col_len, vector<char>(num_cols));

    for (int col = 0; col < num_cols; ++col) {
        for (int row = 0; row < col_len; ++row) {
            matrix[row][col] = text[col * col_len + row];
        }
    }
    return matrix;
}

void processPermutation(const vector<vector<char>>& matrix, const vector<int>& order) {
    static int count = 0;
    if (++count % 100000 == 0) cout << "." << flush;
}

void writeColumnsToFile(const vector<vector<char>>& matrix, const string& filename) {
    ofstream out(filename);
    if (!out.is_open()) throw runtime_error("Kon output file niet aanmaken");

    int num_cols = matrix[0].size();

    // Header
    for (int i = 0; i < num_cols; ++i) out << "KOLOM " << i << "    ";
    out << endl;
    for (int i = 0; i < num_cols; ++i) out << "-----------";
    out << endl;

    // Data
    for (int r = 0; r < matrix.size(); ++r) {
        for (int c = 0; c < num_cols; ++c) {
            out << matrix[r][c] << "          ";
        }
        out << endl;
    }
    cout << "Kolommen weggeschreven naar " << filename << endl;
}

void generatePermutations(vector<vector<char>>& matrix) {
    int num_cols = matrix[0].size();
    vector<int> indices(num_cols);
    for (int i = 0; i < num_cols; ++i) indices[i] = i;

    int count = 0;
    do {
        vector<vector<char>> permuted(matrix.size(), vector<char>(num_cols));
        for (int r = 0; r < matrix.size(); ++r) {
            for (int c = 0; c < num_cols; ++c) {
                permuted[r][c] = matrix[r][indices[c]];
            }
        }
        processPermutation(permuted, indices);
        count++;
    } while (next_permutation(indices.begin(), indices.end()));

    cout << "\nTotaal: " << count << " permutaties" << endl;
}

int main() {
    try {
        string text = readFile("../viginereplus/01-OPGAVE-viginereplus.txt");
        cout << "Ingelezen: " << text.length() << " tekens" << endl;

        vector<vector<char>> matrix = textToMatrix(text, 10);
        cout << "Matrix: " << matrix.size() << " x " << matrix[0].size() << endl;

        writeColumnsToFile(matrix, "../viginereplus/output_columns.txt");

        cout << "Genereren permutaties..." << endl;
        generatePermutations(matrix);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}