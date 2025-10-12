#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

// Functie om de kolommen naar een bestand te schrijven
void writeColumnsToFile(const vector<string>& columns, const string& filename, int num_columns) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cout << "Error: Kon output file niet aanmaken." << endl;
        return;
    }

    // Bepaal het aantal rijen op basis van de eerste kolom (ervan uitgaande dat alle kolommen even lang zijn)
    int num_rows = columns.empty() ? 0 : columns[0].length();

    // Header
    for (int i = 0; i < num_columns; ++i) {
        outFile << "KOLOM " << i << "    ";
    }
    outFile << endl;

    for (int i = 0; i < num_columns; ++i) {
        outFile << "-----------";
    }
    outFile << endl;

    // Loop door elke rij
    for (int i = 0; i < num_rows; ++i) {
        // Loop door elke kolom
        for (int j = 0; j < num_columns; ++j) {
            // Controleer of de huidige rij en kolom geldig zijn
            if (j < columns.size() && i < columns[j].length()) {
                outFile << columns[j][i] << "          ";
            } else {
                outFile << "           "; // Lege ruimte voor kolommen met minder rijen
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

        // Pas hier het aantal kolommen aan
        int num_columns = 9;
        vector<string> columns(num_columns);

        for (int i = 0; i < ciphertext.length(); ++i) {
            columns[i % num_columns] += ciphertext[i];
        }

        writeColumnsToFile(columns, "../viginereplus/output_columns.txt", num_columns);

    } else {
        cout << "Error: Kon het bestand niet openen op pad: " << file_path << endl;
        return 1;
    }

    return 0;
}