#include "audioengine.h"

#include <memory>

#include <QAudioDeviceInfo>
#include <QDebug>
#include <QFile>
#include <QDir>


AudioEngine::AudioEngine(Visualizer::Constants::queue_t& eventQueue, QObject* parent)
    : QObject(parent)
    , m_toneBuffer (new char[s_toneBufferSize])
    , m_eventQueue(eventQueue)
{
    m_toneTimer.setParent(this);
    m_fileWriter.setParent(this);
    m_decoder.setParent(this);
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
    m_format.setChannelCount(2);
    m_format.setCodec("audio/pcm");
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setSampleRate(48000);
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
    m_audioOutput->setVolume(0.5);

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

        if (m_bufferQueue.isEmpty())
        {
            m_bufferQueue.enqueue(buffer);
            processQueue();
        }
        else
        {
            m_bufferQueue.enqueue(buffer);
        }

    });

    QObject::connect(&m_decoder, &QAudioDecoder::finished, [this]()
    {
        if (m_writeWavFile && !m_fileWriter.close())
        {
            qWarning() << "Did not / could not write output file" << endl;
        }
    });

}


void AudioEngine::startToneGenerator()
{
    setup();

    m_generator->start();
    m_audioOutput->stop();

    m_device = m_audioOutput->start();

    m_toneTimer.setInterval(10);
    QObject::connect(&m_toneTimer, &QTimer::timeout, [&]()
    {
        m_generator->readData(m_toneBuffer, s_toneBufferSize);

        QByteArray audioData;
        audioData.setRawData(m_toneBuffer, s_toneBufferSize);

        QAudioBuffer buffer(audioData, m_format);

        if (m_bufferQueue.isEmpty())
        {
            m_bufferQueue.enqueue(buffer);
            processQueue();
        }
        else
        {
            m_bufferQueue.enqueue(buffer);
        }
    });

    m_toneTimer.start();

}


void AudioEngine::stop()
{
    m_generator->stop();
    m_toneTimer.stop();
    m_decoder.stop();
    m_audioOutput->stop();

    emit stopped();
}


void AudioEngine::startPlayback()
{
    setup();

    m_audioOutput->stop();

    // TODO: Fix this, for some reason we can't set the resource file directly in the
    // decoder, so we copy the file from the resources into the current directory

    QFile file(":/test.mp3");
    file.copy("test.mp3");

    m_decoder.setSourceFilename(QDir::currentPath() +"/test.mp3");
    m_device = m_audioOutput->start();
    m_decoder.start();
}


void AudioEngine::processQueue()
{
    while (!m_bufferQueue.isEmpty())
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
                    qint64 written = m_device->write(static_cast<const char*>(buffer.data()) + totalBytesWritten, length);

                    if (m_writeWavFile)
                    {
                        m_fileWriter.write(static_cast<const char*>(buffer.data()) + totalBytesWritten, length);
                    }

                    Visualizer::Constants::Event event;

                    const unsigned eventLength = std::min(unsigned(length), Visualizer::Constants::bufferSize);
                    event.nrElements = eventLength;

                    for (int i = 0; i < eventLength; ++i)
                    {
                        event.data[i] = static_cast<const int*>(buffer.data())[i];
                    }

                    m_eventQueue.push(event);

                    bytesRemaining -= written;
                    totalBytesWritten += written;
                }

                --chunks;
            }
        }
    }
}
