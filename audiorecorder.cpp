
#include "audiorecorder.h"

AudioRecorder::AudioRecorder()
{

    inputDevice = QMediaDevices::defaultAudioInput();
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    if (!inputDevice.isFormatSupported(format))
    {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = inputDevice.preferredFormat();
    }

    audioInput = new QAudioSource(inputDevice, format);
}

AudioRecorder::~AudioRecorder()
{
    delete audioInput;
}

void AudioRecorder::startRecording()
{
    device = audioInput->start();
    qDebug() << "🎤 Enregistrement en cours...";
    qDebug() << "🎤 Format :" << format.sampleRate() << "Hz, " << format.channelCount() << "canaux, " << format.sampleFormat();
}

void AudioRecorder::stopRecording()
{
    audioInput->stop();
    qDebug() << " Enregistrement terminé...";
    qDebug() << "⏹️ Format :" << format.sampleRate() << "Hz, " << format.channelCount() << "canaux, " << format.sampleFormat();
}