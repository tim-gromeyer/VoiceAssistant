cd "$(dirname "$0")"
cd ..

lupdate src/*.cpp "$(find plugins/* -name '*.cpp')" plugins/*.h "$(find speechtotext/* -name '*.cpp')" ui/*.ui -ts translations/VoiceAssistant_de.ts -noobsolete
