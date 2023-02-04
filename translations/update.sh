cd $(dirname "$0")
cd ..

lupdate src/*.cpp  ui/*.ui -ts translations/VoiceAssistant_de_DE.ts -noobsolete
