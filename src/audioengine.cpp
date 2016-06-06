#include "audioengine.h"

#include <memory>

#include <QAudioDeviceInfo>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QDebug>


#define debugChunks QNoDebug()


AudioEngine::AudioEngine(Visualizer::Common::fft_queue_t& eventQueue,
                         Visualizer::Common::sample_queue_t& sampleEventQueue,
                         QObject* parent)
    : QObject(parent)
    , m_toneBuffer (new char[s_toneBufferSize])
    , m_fftEventQueue(eventQueue)
    , m_sampleEventQueue(sampleEventQueue)
{
    m_toneTimer.setParent(this);
    m_fileWriter.setParent(this);
    m_decoder.setParent(this);
    m_audioOutputTimer.setParent(this);
}


AudioEngine::~AudioEngine()
{
    stop();

    if (m_toneBuffer)
    {
        delete[] m_toneBuffer;
        m_toneBuffer = nullptr;
    }
}


void AudioEngine::setup()
{
    m_format.setChannelCount(1);
    m_format.setCodec("audio/pcm");
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setSampleRate(Visualizer::Common::sampleRate);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleSize(16);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }

    m_decoder.setAudioFormat(m_format);

    if (m_generator)
    {
        m_generator->deleteLater();
    }

    m_generator = new ToneGenerator(m_format, qint64(1000000), 600, this);
    m_generator->start();

    if (m_audioOutput)
    {
        m_audioOutput->deleteLater();
    }

    m_audioOutput = new QAudioOutput(m_format, this);
    m_audioOutput->setVolume(1.0);

    QObject::connect(m_audioOutput, &QAudioOutput::stateChanged, [this](QAudio::State newState)
    {
        switch (newState)
        {
            case QAudio::IdleState:
                qDebug() << "Idle";
                break;

            case QAudio::StoppedState:
                qDebug() << "Stopped";
                if (m_audioOutput->error() != QAudio::NoError)
                {
                    qDebug() << "Error:" << m_audioOutput->error();
                }
                break;

            default:
                break;
        }
    });


    QObject::connect(&m_decoder,
                     static_cast<void(QAudioDecoder::*)(QAudioDecoder::Error)>(&QAudioDecoder::error),
                     [this](QAudioDecoder::Error error)
    {
        qDebug() << m_decoder.errorString();
    });

    QObject::connect(&m_decoder, &QAudioDecoder::bufferReady, [this]()
    {
        const auto& buffer = m_decoder.read();

        if (!buffer.isValid())
        {
            qWarning() << "Invalid buffer!";
            return;
        }

        if (m_writeWavFile && !m_fileWriter.isOpen() && !m_fileWriter.open("file.wav", buffer.format()))
        {
            qWarning() << "Can't open file writer";
            return;
        }

        m_bufferQueue.enqueue(buffer);

    });


    QObject::connect(&m_decoder, &QAudioDecoder::finished, [this]()
    {
        m_audioOutputTimer.start();
    });



    QObject::connect(&m_decoder, &QAudioDecoder::finished, [this]()
    {
        if (m_writeWavFile && !m_fileWriter.close())
        {
            qWarning() << "Did not / could not write output file" << endl;
        }
    });

    m_audioOutputTimer.setTimerType(Qt::PreciseTimer);
    m_audioOutputTimer.setInterval(Visualizer::Common::audioTimerInterval);
    m_audioOutputTimer.setSingleShot(false);
    QObject::connect(&m_audioOutputTimer, &QTimer::timeout, this, &AudioEngine::processQueue);
}


void AudioEngine::startToneGenerator()
{
    setup();

    m_generator->start();
    m_audioOutput->stop();

    m_device = m_audioOutput->start();

    m_toneTimer.setTimerType(Qt::PreciseTimer);
    m_toneTimer.setInterval(15);
    QObject::connect(&m_toneTimer, &QTimer::timeout, [&]()
    {
        m_generator->readData(m_toneBuffer, s_toneBufferSize);

        QByteArray audioData;
        audioData.setRawData(m_toneBuffer, s_toneBufferSize);

        QAudioBuffer buffer(audioData, m_format);
        m_bufferQueue.enqueue(buffer);
    });

    m_toneTimer.start();
    m_audioOutputTimer.start();
}


void AudioEngine::stop()
{
    m_generator->stop();
    m_toneTimer.stop();
    m_audioOutputTimer.stop();
    m_decoder.stop();
    m_audioOutput->stop();

    emit stopped();
}


void AudioEngine::startPlayback(const QString& filePath)
{
    setup();

    m_audioOutput->stop();

    if (filePath.isEmpty())
    {
        // TODO: Fix this, for some reason we can't set the resource file directly in the
        // decoder, so we copy the file from the resources into the current directory

        QFile file(":/test.mp3");
     // QFile file(":/440hz.wav");
        file.copy("test.mp3");

        m_decoder.setSourceFilename(QDir::currentPath() +"/test.mp3");
    }
    else
    {
        m_decoder.setSourceFilename(filePath);
    }


    m_device = m_audioOutput->start();
    m_decoder.start();

}

template<class DestinationQueue, class DestinationEvent>
void AudioEngine::dispatchInChunks(const QByteArray& buffer,
                                   unsigned chunkSize,
                                   QByteArray& partialBuffer,
                                   DestinationEvent& event,
                                   DestinationQueue& queue)
{
    debugChunks << "____________dispatchInChunks________________________";
    debugChunks << "Current buffer size:" << QAudioBuffer(buffer, m_format).sampleCount();

    partialBuffer.append(buffer);

    QAudioBuffer totalAudioBuffer(partialBuffer, m_format);

    debugChunks << "Total size added to previous:" << totalAudioBuffer.sampleCount();

    if (totalAudioBuffer.sampleCount() < chunkSize)
    {
        debugChunks << "Not enough, saving for next round!";
        return;
    }

    unsigned nrChunks = totalAudioBuffer.sampleCount() / chunkSize;
    unsigned lastIndex = chunkSize * nrChunks;

    debugChunks << "Nr chunks:" << nrChunks << " last index:" << lastIndex;

    event.nrChannels = m_format.channelCount();
    const Visualizer::Common::sample_t *samples = totalAudioBuffer.constData<Visualizer::Common::sample_t>();

    for(int i = 0; i < lastIndex; ++i)
    {
        event.data[event.nrElements++] = samples[i];
        if (event.nrElements == chunkSize)
        {
            debugChunks << "Sending event";

            queue.push(event);
            event.nrElements = 0;
        }
    }

    debugChunks << "Removed from 0 to " << lastIndex << " remaining: " << totalAudioBuffer.sampleCount() % chunkSize;

    partialBuffer =  partialBuffer.remove(0, lastIndex * sizeof(Visualizer::Common::sample_t));

}


void AudioEngine::sendEvents(const QByteArray& buffer)
{
    dispatchInChunks<Visualizer::Common::fft_queue_t,
                     Visualizer::Common::FFTEvent>(buffer, Visualizer::Common::fftNrSamples,
                                                   m_partialFFTBuffer, m_fftEvent, m_fftEventQueue);


    dispatchInChunks<Visualizer::Common::sample_queue_t,
                     Visualizer::Common::SampleEvent>(buffer, Visualizer::Common::nrSamples,
                                                      m_partialSampleBuffer, m_sampleEvent, m_sampleEventQueue);
}


void AudioEngine::processQueue()
{
    if (!m_bufferQueue.isEmpty())
    {
        const auto& buffer = m_bufferQueue.dequeue();

        qint64 bytesRemaining = buffer.byteCount();
        qint64 totalBytesWritten = 0;

        while (bytesRemaining > 0)
        {
            if (m_audioOutput->state() == QAudio::StoppedState)
            {
                return;
            }

            int chunks = m_audioOutput->bytesFree() / m_audioOutput->periodSize();

            while (chunks > 0 && bytesRemaining > 0)
            {
                const qint64 length = qMin(bytesRemaining, qint64(m_audioOutput->periodSize()));
                if (length > 0)
                {
                    const char* currentPosition = static_cast<const char*>(buffer.constData()) + totalBytesWritten;
                    const unsigned sampleSize = m_format.bytesPerFrame() / m_format.channelCount();
                    const unsigned nrSamples = length / sampleSize;

                    if (m_writeWavFile)
                    {
                        m_fileWriter.write(currentPosition, length);
                    }

                    QByteArray rawBuffer;
                    QDataStream stream(&rawBuffer, QIODevice::WriteOnly);
                    qint64 written = stream.writeRawData(currentPosition, nrSamples * sampleSize);

                    QAudioBuffer audioBuffer(rawBuffer, m_format);
                    audioBuffer.duration();

                    sendEvents(rawBuffer);

                    written = m_device->write(rawBuffer);

                    bytesRemaining -= written;
                    totalBytesWritten += written;
                }

                --chunks;
            }
        }
    }

}
