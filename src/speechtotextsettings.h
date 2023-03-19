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

class SpeechToTextSettings : public SettingsWidget
{
    Q_OBJECT
public:
    explicit SpeechToTextSettings(QTextToSpeech *tts, QWidget *parent = nullptr);
    ~SpeechToTextSettings() = default;

    void apply() override;
    void finish() override;

private:
    void setupUi();
    void populateComboBoxes();

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
};
