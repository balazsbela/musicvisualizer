#pragma once

#include "fftwrapper.h"
#include "tonegenerator.h"
#include "wavefilewriter.h"

#include <QAudioFormat>
#include <QAudioDecoder>
#include <QAudioOutput>
#include <QScopedPointer>
#include <QQueue>


class AudioEngine : QObject
{
    Q_OBJECT

public:

    AudioEngine(QObject* parent = nullptr);
    ~AudioEngine() = default;

    void setup();
    void startPlayback();
    void startToneGenerator();

private:

    void processQueue();

    QAudioFormat                     m_format;
    QAudioDecoder                    m_decoder;
    QIODevice*                       m_device;
    QAudioOutput*                    m_audioOutput = nullptr;
    ToneGenerator*                   m_generator = nullptr;
    QQueue<QAudioBuffer>             m_bufferQueue;

    WaveFileWriter                   m_fileWriter;
    FFTWrapper                       m_fft;

    bool                             m_writeWavFile = false;

};
