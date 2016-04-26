#pragma once

#include "fftwrapper.h"
#include "tonegenerator.h"
#include "wavefilewriter.h"

#include <QAudioFormat>
#include <QAudioDecoder>
#include <QAudioOutput>
#include <QScopedPointer>
#include <QQueue>
#include <QTimer>


class AudioEngine : public QObject
{
    Q_OBJECT

public:

    AudioEngine(QObject* parent = nullptr);
    ~AudioEngine();

    void setup();
    void startPlayback();
    void startToneGenerator();
    void stop();

private:

    void processQueue();

    static const unsigned            s_toneBufferSize = 20 * 512;

    QAudioFormat                     m_format;
    QAudioDecoder                    m_decoder;
    QIODevice*                       m_device = nullptr;
    QAudioOutput*                    m_audioOutput = nullptr;
    ToneGenerator*                   m_generator = nullptr;
    QQueue<QAudioBuffer>             m_bufferQueue;

    WaveFileWriter                   m_fileWriter;
    FFTWrapper                       m_fft;

    bool                             m_writeWavFile = false;

    QTimer                           m_toneTimer;
    char*                            m_toneBuffer = nullptr;

};
