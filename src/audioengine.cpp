#include "audioengine.h"

#include <memory>

#include <QAudioDeviceInfo>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QDebug>


#define debugFFTChunks QNoDebug()


AudioEngine::AudioEngine(Visualizer::Constants::sample_queue_t& eventQueue, QObject* parent)
    : QObject(parent)
    , m_toneBuffer (new char[s_toneBufferSize])
    , m_eventQueue(eventQueue)
{
    m_toneTimer.setParent(this);
    m_fileWriter.setParent(this);
    m_decoder.setParent(this);

    m_audioOutputTimer.setParent(this);
    m_audioOutputTimer.setInterval(15);
    m_audioOutputTimer.setSingleShot(false);
    QObject::connect(&m_audioOutputTimer, &QTimer::timeout, this, &AudioEngine::processQueue, Qt::QueuedConnection);
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
    m_format.setSampleRate(Visualizer::Constants::sampleRate);
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


void AudioEngine::startPlayback()
{
    setup();

    m_audioOutput->stop();

    // TODO: Fix this, for some reason we can't set the resource file directly in the
    // decoder, so we copy the file from the resources into the current directory

    QFile file(":/test.mp3");
//    QFile file(":/440hz.wav");
    file.copy("test.mp3");

    m_decoder.setSourceFilename(QDir::currentPath() +"/test.mp3");
    m_device = m_audioOutput->start();
    m_decoder.start();

}


void AudioEngine::sendToFFT(const QByteArray& buffer)
{
    QAudioBuffer current(buffer, m_format);

    debugFFTChunks << "____________sendToFFT________________________";
    debugFFTChunks << "Current buffer size:" << current.sampleCount();

    m_previousFFTBuffer.append(buffer);
    QAudioBuffer totalAudioBuffer(m_previousFFTBuffer, m_format);

    debugFFTChunks << "Total size added to previous:" << totalAudioBuffer.sampleCount();

    if (totalAudioBuffer.sampleCount() < Visualizer::Constants::fftBufferSize)
    {
        debugFFTChunks << "Not enough, saving for next round!";
        return;
    }

    unsigned nrChunks = totalAudioBuffer.sampleCount() / Visualizer::Constants::fftBufferSize;
    unsigned lastIndex = Visualizer::Constants::fftBufferSize * nrChunks;

    debugFFTChunks << "Nr chunks:" << nrChunks << " last index:" << lastIndex;

    m_fftEvent.nrChannels = m_format.channelCount();
    const qint16 *samples = totalAudioBuffer.constData<qint16>();

    for(int i = 0; i < lastIndex; ++i)
    {
        m_fftEvent.data[m_fftEvent.nrElements++] = samples[i];
        if (m_fftEvent.nrElements == Visualizer::Constants::fftBufferSize)
        {
            debugFFTChunks << "Sending fft event";

            m_eventQueue.push(m_fftEvent);
            m_fftEvent.nrElements = 0;
        }
    }

    debugFFTChunks << "Removed from 0 to " << lastIndex << " remaining: " << totalAudioBuffer.sampleCount() % Visualizer::Constants::fftBufferSize;

    m_previousFFTBuffer =  m_previousFFTBuffer.remove(0, lastIndex * sizeof(qint16));
}


void AudioEngine::processQueue()
{
    if (!m_bufferQueue.isEmpty())
    {
        const auto& buffer = m_bufferQueue.dequeue();

        qint64 bytesRemaining = buffer.byteCount();
        qint64 totalBytesWritten = 0;

        Visualizer::Constants::sample_t sample = 0.0f;
        const QByteArray byteArray(static_cast<const char*>(buffer.constData()),  buffer.byteCount());
        QDataStream instream(byteArray);
        instream.setFloatingPointPrecision(QDataStream::SinglePrecision);

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

                    sendToFFT(rawBuffer);

                    QTimer::singleShot(0, this, [this, &written, rawBuffer]()
                    {
                        written = m_device->write(rawBuffer);
                    });

                    m_audioOutputTimer.setInterval(audioBuffer.duration() / 1000);


                    bytesRemaining -= written;
                    totalBytesWritten += written;
                }

                --chunks;
            }
        }
    }
}
