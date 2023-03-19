#include "speechtotextsettings.h"

#include <QComboBox>
#include <QCommandLinkButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QSlider>
#include <QTextToSpeech>
#include <QVBoxLayout>

Q_DECLARE_METATYPE(QVoice);

SpeechToTextSettings::SpeechToTextSettings(QTextToSpeech *tts, QWidget *parent)
    : SettingsWidget(parent)
    , m_tts(tts)
{
    setDisplayCategory(tr("General settings"));
    setDisplayName(tr("Speech to text"));

    setupUi();
    populateComboBoxes();
}

void SpeechToTextSettings::apply() {}

void SpeechToTextSettings::finish() {}

void SpeechToTextSettings::setupUi()
{
    verticalLayout = new QVBoxLayout(this);
    settingsGroup = new QGroupBox(this);
    formLayout = new QFormLayout(settingsGroup);
    label = new QLabel(settingsGroup);
    formLayout->setWidget(0, QFormLayout::LabelRole, label);

    engineComboBox = new QComboBox(settingsGroup);
    formLayout->setWidget(0, QFormLayout::FieldRole, engineComboBox);

    languageLabel = new QLabel(settingsGroup);
    formLayout->setWidget(1, QFormLayout::LabelRole, languageLabel);

    languageComboBox = new QComboBox(settingsGroup);
    formLayout->setWidget(1, QFormLayout::FieldRole, languageComboBox);

    voiceLabel = new QLabel(settingsGroup);
    formLayout->setWidget(2, QFormLayout::LabelRole, voiceLabel);

    voiceComboBox = new QComboBox(settingsGroup);
    formLayout->setWidget(2, QFormLayout::FieldRole, voiceComboBox);

    pitchLabel = new QLabel(settingsGroup);
    formLayout->setWidget(3, QFormLayout::LabelRole, pitchLabel);

    pitchSlider = new QSlider(settingsGroup);
    pitchSlider->setMinimum(-10);
    pitchSlider->setMaximum(10);
    pitchSlider->setOrientation(Qt::Horizontal);

    formLayout->setWidget(3, QFormLayout::FieldRole, pitchSlider);

    rateLabel = new QLabel(settingsGroup);
    formLayout->setWidget(4, QFormLayout::LabelRole, rateLabel);

    rateSlider = new QSlider(settingsGroup);
    rateSlider->setMinimum(-10);
    rateSlider->setMaximum(10);
    rateSlider->setOrientation(Qt::Horizontal);
    formLayout->setWidget(4, QFormLayout::FieldRole, rateSlider);

    verticalLayout->addWidget(settingsGroup);

    testGroup = new QGroupBox(this);
    verticalLayout_2 = new QVBoxLayout(testGroup);
    testTextEdit = new QPlainTextEdit(testGroup);
    verticalLayout_2->addWidget(testTextEdit);

    testButton = new QCommandLinkButton(testGroup);
    verticalLayout_2->addWidget(testButton);

    verticalLayout->addWidget(testGroup);

    settingsGroup->setTitle(tr("Settings"));
    label->setText(tr("Engine"));
    languageLabel->setText(tr("Language"));
    voiceLabel->setText(tr("Voice"));
    pitchLabel->setText(tr("Pitch"));
    rateLabel->setText(tr("Rate"));
    testGroup->setTitle(tr("Test"));
    testTextEdit->setPlaceholderText(tr("Enter some text here to test the settings"));
    testButton->setText(tr("Say", nullptr));
}

void SpeechToTextSettings::populateComboBoxes()
{
    if (!m_tts)
        return;

    engineComboBox->addItems(QTextToSpeech::availableEngines());
    for (auto &locale : m_tts->availableLocales()) {
        languageComboBox->addItem(locale.nativeLanguageName(), locale);
    }
    languageComboBox->setCurrentText(m_tts->locale().nativeLanguageName());
    for (auto &voice : m_tts->availableVoices()) {
        voiceComboBox->addItem(voice.name(), QVariant::fromValue(voice));
    }
    voiceComboBox->setCurrentText(m_tts->voice().name());
    pitchSlider->setValue(int(m_tts->pitch() * 10));
    rateSlider->setValue(int(m_tts->rate() * 10));
}
