#pragma once

#include "tonegenerator.h"
#include "wavefilewriter.h"
#include "constants.h"

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

    explicit AudioEngine(Visualizer::Constants::sample_queue_t& eventQueue, QObject* parent = nullptr);
    ~AudioEngine();

    void setup();

public slots:

    void startPlayback();
    void startToneGenerator();
    void stop();

signals:

    void stopped();

private:

    void processQueue();
    void sendToFFT(const auto& buffer, const qint64 length);


    static const unsigned            s_toneBufferSize = 20 * 512;

    QAudioFormat                     m_format;
    QAudioDecoder                    m_decoder;
    QIODevice*                       m_device = nullptr;
    QAudioOutput*                    m_audioOutput = nullptr;
    ToneGenerator*                   m_generator = nullptr;
    QQueue<QAudioBuffer>             m_bufferQueue;

    WaveFileWriter                   m_fileWriter;

    bool                             m_writeWavFile = false;

    QTimer                           m_toneTimer;
    char*                            m_toneBuffer = nullptr;

    Visualizer::Constants::sample_queue_t&  m_eventQueue;
};
