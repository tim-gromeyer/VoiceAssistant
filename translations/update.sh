cd "$(dirname "$0")"
cd ..

cd "$(dirname "$0")"
cd ..

cd "$(dirname "$0")"
cd ..

for ts_file in translations/VoiceAssistant_*.ts; do
    lupdate src/*.cpp "$(find plugins/* -name '*.cpp')" plugins/*.h "$(find speechtotext/* -name '*.cpp')" ui/*.ui -ts "$ts_file" -noobsolete
done
