#include "texttospeechsettings.h"
#include "utils.h"

#include <QComboBox>
#include <QCommandLinkButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QPlainTextEdit>
#include <QSettings>
#include <QSlider>
#include <QTextToSpeech>
#include <QVBoxLayout>

TextToSpeechSettings::TextToSpeechSettings(QTextToSpeech *tts, QWidget *parent)
    : SettingsWidget(parent)
    , m_tts(tts)
{
    setDisplayCategory(tr("General"));
    setDisplayName(tr("Text to speech"));
    setCategoryIcon(QIcon::fromTheme(QStringLiteral("settings")));

    setupUi();
    populateComboBoxes();
}

void TextToSpeechSettings::apply()
{
    if (!m_tts || !m_newtts)
        return;

    if (m_newtts == m_tts)
        return;

    m_newtts->moveToThread(m_tts->thread());
    m_tts = m_newtts;
    Q_EMIT setNewTTS(m_tts);
}

void TextToSpeechSettings::finish()
{
    if (!settings())
        return;

    settings()->beginGroup(QStringLiteral("TextToSpeech"));
    settings()->setValue(QStringLiteral("Engine"), engineComboBox->currentText());
    settings()->setValue(QStringLiteral("Language"), languageComboBox->currentData());
    settings()->setValue(QStringLiteral("Voice"), voiceComboBox->currentText());
    settings()->setValue(QStringLiteral("Pitch"), pitchSlider->value());
    settings()->setValue(QStringLiteral("Rate"), rateSlider->value());
    settings()->endGroup();
}

void TextToSpeechSettings::setupUi()
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
    connect(testButton, &QCommandLinkButton::clicked, this, &TextToSpeechSettings::say);

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

void TextToSpeechSettings::populateComboBoxes(bool useNewTTS)
{
    if (useNewTTS && !m_newtts)
        return;
    else if (!m_tts)
        return;

    QSignalBlocker blocker1(languageComboBox);
    QSignalBlocker blocker2(voiceComboBox);

    languageComboBox->clear();
    voiceComboBox->clear();

    if (engineComboBox->count() == 0) {
        for (const QString &engine : QTextToSpeech::availableEngines())
            if (engine != utils::strings::literals::L1("mock"))
                engineComboBox->addItem(engine);
    }
    for (auto &locale : useNewTTS ? m_newtts->availableLocales() : m_tts->availableLocales()) {
        languageComboBox->addItem(locale.nativeLanguageName(), locale);
    }
    languageComboBox->setCurrentText(useNewTTS ? m_newtts->locale().nativeLanguageName()
                                               : m_tts->locale().nativeLanguageName());
    for (auto &voice : useNewTTS ? m_newtts->availableVoices() : m_tts->availableVoices()) {
        voiceComboBox->addItem(voice.name(), QVariant::fromValue(voice));
    }
    voiceComboBox->setCurrentText(useNewTTS ? m_newtts->voice().name() : m_tts->voice().name());
    pitchSlider->setValue(int(useNewTTS ? m_newtts->pitch() : m_tts->pitch() * 10));
    rateSlider->setValue(int(useNewTTS ? m_newtts->rate() : m_tts->rate() * 10));

    // Sort the combo boxes
    engineComboBox->model()->sort(0);
    languageComboBox->model()->sort(0);
    voiceComboBox->model()->sort(0);

    connect(engineComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &TextToSpeechSettings::onEngineChanged,
            Qt::UniqueConnection);
    connect(languageComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &TextToSpeechSettings::onLanguageChanged,
            Qt::UniqueConnection);
    connect(voiceComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &TextToSpeechSettings::onVoiceChanged,
            Qt::UniqueConnection);
    connect(pitchSlider,
            &QSlider::sliderMoved,
            this,
            &TextToSpeechSettings::onPitchChanged,
            Qt::UniqueConnection);
    connect(rateSlider,
            &QSlider::sliderMoved,
            this,
            &TextToSpeechSettings::onRateChanged,
            Qt::UniqueConnection);
}

void TextToSpeechSettings::onEngineChanged()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    setupTTS();
    populateComboBoxes(true);

    QGuiApplication::restoreOverrideCursor();
}
void TextToSpeechSettings::onLanguageChanged()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    setupTTS(false);

    m_newtts->setLocale(languageComboBox->currentData().toLocale());
    populateComboBoxes(true);

    QGuiApplication::restoreOverrideCursor();
}
void TextToSpeechSettings::onVoiceChanged()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    setupTTS(false);

    m_newtts->setVoice(qvariant_cast<QVoice>(voiceComboBox->currentData()));
    populateComboBoxes(true);

    QGuiApplication::restoreOverrideCursor();
}
void TextToSpeechSettings::onPitchChanged()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    setupTTS(false);

    m_newtts->setPitch(float(pitchSlider->value() / 10));

    QGuiApplication::restoreOverrideCursor();
}
void TextToSpeechSettings::onRateChanged()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    setupTTS(false);

    m_newtts->setRate(float(rateSlider->value() / 10));

    QGuiApplication::restoreOverrideCursor();
}

void TextToSpeechSettings::onTTSChanged()
{
    // populateComboBoxes(true);
}

void TextToSpeechSettings::say()
{
    if (m_newtts)
        m_newtts->say(testTextEdit->toPlainText());
    else if (m_tts)
        m_tts->say(testTextEdit->toPlainText());
}

void TextToSpeechSettings::setupTTS(bool override)
{
    if (override || !m_newtts)
        m_newtts = new QTextToSpeech(engineComboBox->currentText());

#if QT6
    connect(m_newtts,
            &QTextToSpeech::engineChanged,
            this,
            &TextToSpeechSettings::onTTSChanged,
            Qt::UniqueConnection);
#endif
    connect(m_newtts,
            &QTextToSpeech::localeChanged,
            this,
            &TextToSpeechSettings::onTTSChanged,
            Qt::UniqueConnection);
    connect(m_newtts,
            &QTextToSpeech::voiceChanged,
            this,
            &TextToSpeechSettings::onTTSChanged,
            Qt::UniqueConnection);
    connect(m_newtts,
            &QTextToSpeech::pitchChanged,
            this,
            &TextToSpeechSettings::onTTSChanged,
            Qt::UniqueConnection);
    connect(m_newtts,
            &QTextToSpeech::rateChanged,
            this,
            &TextToSpeechSettings::onTTSChanged,
            Qt::UniqueConnection);
}
