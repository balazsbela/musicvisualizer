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

    explicit AudioEngine(Visualizer::Common::fft_queue_t& eventQueue,
                         Visualizer::Common::sample_queue_t& sampleEventQueue,
                         QObject* parent = nullptr);
    ~AudioEngine();

    void setup();

public slots:

    void startPlayback(const QString& filePath = {});
    void startToneGenerator();
    void stop();

signals:

    void stopped();

private slots:

    void processQueue();

private:

    void sendEvents(const QByteArray& buffer);

    template<class DestinationQueue, class DestinationEvent>
    void dispatchInChunks(const QByteArray& buffer,
                          unsigned chunkSize,
                          QByteArray& partialBuffer,
                          DestinationEvent& event,
                          DestinationQueue& queue);

    static const unsigned            s_toneBufferSize = 20 * 512;

    QAudioFormat                     m_format;
    QAudioDecoder                    m_decoder;
    QIODevice*                       m_device = nullptr;
    QAudioOutput*                    m_audioOutput = nullptr;
    ToneGenerator*                   m_generator = nullptr;
    QQueue<QAudioBuffer>             m_bufferQueue;
    QTimer                           m_audioOutputTimer;

    WaveFileWriter                   m_fileWriter;

    bool                             m_writeWavFile = false;

    QTimer                           m_toneTimer;
    char*                            m_toneBuffer = nullptr;

    Visualizer::Common::fft_queue_t&        m_fftEventQueue;
    Visualizer::Common::FFTEvent            m_fftEvent;
    QByteArray                              m_partialFFTBuffer;


    Visualizer::Common::sample_queue_t&     m_sampleEventQueue;
    Visualizer::Common::SampleEvent         m_sampleEvent;
    QByteArray                              m_partialSampleBuffer;

};
