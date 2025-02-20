#pragma once

#include <plugins/settingswidget.h>
#include <QWidget>

class QComboBox;
class QCommandLinkButton;
class QFormLayout;
class QGroupBox;
class QPlainTextEdit;
class QSlider;
class QVBoxLayout;
class QTextToSpeech;

// TODO: Improve text to speech settings. It's far from perfect
class TextToSpeechSettings : public SettingsWidget
{
    Q_OBJECT
public:
    explicit TextToSpeechSettings(QTextToSpeech *tts, QWidget *parent = nullptr);
    ~TextToSpeechSettings() = default;

    void setup() override{};
    void apply() override;
    void finish() override;

Q_SIGNALS:
    void setNewTTS(QTextToSpeech *);

private Q_SLOTS:
    void onEngineChanged();
    void onLanguageChanged();
    void onVoiceChanged();
    void onPitchChanged();
    void onRateChanged();

    void say();

    void onTTSChanged();

private:
    void setupUi();
    void populateComboBoxes(bool useNewTTS = false);
    void setupTTS(bool override = true);

    QComboBox *engineComboBox = nullptr;
    QComboBox *languageComboBox = nullptr;
    QComboBox *voiceComboBox = nullptr;
    QCommandLinkButton *testButton = nullptr;
    QFormLayout *formLayout = nullptr;
    QGroupBox *settingsGroup = nullptr;
    QGroupBox *testGroup = nullptr;
    QLabel *label = nullptr;
    QLabel *languageLabel = nullptr;
    QLabel *pitchLabel = nullptr;
    QLabel *rateLabel = nullptr;
    QLabel *voiceLabel = nullptr;
    QPlainTextEdit *testTextEdit = nullptr;
    QSlider *pitchSlider = nullptr;
    QSlider *rateSlider = nullptr;
    QVBoxLayout *verticalLayout = nullptr;
    QVBoxLayout *verticalLayout_2 = nullptr;

    QTextToSpeech *m_tts = nullptr;
    QTextToSpeech *m_newtts = nullptr;
};
