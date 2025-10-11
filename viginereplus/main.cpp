#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

// Functie om de kolommen naar een bestand te schrijven
void writeColumnsToFile(const vector<string>& columns, const string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cout << "Error: Kon output file niet aanmaken." << endl;
        return;
    }

    int num_rows = columns[0].length();

    // Header
    outFile << "KOLOM 0    KOLOM 1    KOLOM 2    KOLOM 3    KOLOM 4    KOLOM 5    KOLOM 6    KOLOM 7    KOLOM 8    KOLOM 9" << endl;
    outFile << "----------------------------------------------------------------------------------------------------------" << endl;

    // Loop door elke rij
    for (int i = 0; i < num_rows; ++i) {
        // Loop door elke kolom
        for (int j = 0; j < columns.size(); ++j) {
            outFile << columns[j][i] << "          ";
        }
        outFile << endl;
    }

    cout << "Kolommen succesvol weggeschreven naar " << filename << endl;
}

int main() {
    string file_path = "../viginereplus/01-OPGAVE-viginereplus.txt";

    ifstream file(file_path);
    string ciphertext;

    if (file.is_open()) {
        stringstream buffer;
        buffer << file.rdbuf();
        ciphertext = buffer.str();
        file.close();

        cout << "File succesvol ingelezen!" << endl;
        cout << "Totaal aantal tekens: " << ciphertext.length() << endl;

        int num_columns = 10;
        int column_length = 139;
        vector<string> columns;

        for (int i = 0; i < num_columns; ++i) {
            string col = ciphertext.substr(i * column_length, column_length);
            columns.push_back(col);
        }

        writeColumnsToFile(columns, "../viginereplus/output_columns.txt");

    } else {
        cout << "Error: Kon het bestand niet openen op pad: " << file_path << endl;
        return 1;
    }

    return 0;
}