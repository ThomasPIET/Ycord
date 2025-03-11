#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QIODevice>
#include <QDebug>

class AudioRecorder
{
public:
    AudioRecorder();
    ~AudioRecorder();

    void startRecording();
    void stopRecording();

private:
    QAudioSource *audioInput;
    QIODevice *device;
    QAudioFormat format;
    QAudioDevice inputDevice;
};

#endif // AUDIORECORDER_H
