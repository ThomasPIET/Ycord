cmake_minimum_required(VERSION 3.16)
project(ChatVocal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activer la génération automatique du MOC
set(CMAKE_AUTOMOC ON)

# Trouver Qt6 avec les bons modules
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia Network)

# Ajouter l'exécutable avec tous les fichiers nécessaires
add_executable(ChatVocal main.cpp audioserver.cpp audioclient.cpp audiorecorder.cpp audiomanager.cpp)

# Lier les bibliothèques Qt nécessaires
target_link_libraries(ChatVocal PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::Network)
