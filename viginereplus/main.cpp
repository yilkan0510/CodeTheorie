#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

// Jouw writeColumnsToFile functie blijft ongewijzigd. Perfect voor visualisatie.
void writeColumnsToFile(const vector<string>& columns, const string& filename, int num_columns) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cout << "Error: Kon output file niet aanmaken." << endl;
        return;
    }
    int num_rows = columns.empty() ? 0 : columns[0].length();
    for (int i = 0; i < num_columns; ++i) {
        outFile << "KOLOM " << i << "    ";
    }
    outFile << endl;
    for (int i = 0; i < num_columns; ++i) {
        outFile << "-----------";
    }
    outFile << endl;
    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_columns; ++j) {
            if (j < columns.size() && i < columns[j].length()) {
                outFile << columns[j][i] << "          ";
            } else {
                outFile << "           ";
            }
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
        vector<string> columns(num_columns);

        // --- DIT IS DE ENIGE AANPASSING ---
        // We verdelen de ciphertext nu in 10 blokken van 139 tekens.
        int col_length = ciphertext.length() / num_columns;
        for (int i = 0; i < num_columns; ++i) {
            columns[i] = ciphertext.substr(i * col_length, col_length);
        }
        // ------------------------------------

        // We gebruiken je bestaande functie om het resultaat te bekijken.
        writeColumnsToFile(columns, "../viginereplus/output_columns.txt", num_columns);

    } else {
        cout << "Error: Kon het bestand niet openen op pad: " << file_path << endl;
        return 1;
    }

    return 0;
}