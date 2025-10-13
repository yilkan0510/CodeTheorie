# Stel de minimaal vereiste CMake-versie in.
cmake_minimum_required(VERSION 3.10)

# Definieer de projectnaam en de taal (C++).
project(CodeTheorie)

# Voeg de subdirectories toe aan de build.
# CMake zal in elk van deze mappen zoeken naar een CMakeLists.txt.
add_subdirectory(playfair)
add_subdirectory(viginereplus)