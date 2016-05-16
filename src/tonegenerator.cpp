#include "tonegenerator.h"

#include <qmath.h>
#include <qendian.h>

#include <QDebug>


const qint16  PCMS16MaxValue     =  32767;
const quint16 PCMS16MaxAmplitude =  32768;


qint16 realToPcm(qreal real)
{
    return real * PCMS16MaxValue;
}

void generateSweep(const QAudioFormat &format, QByteArray &buffer)
{
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channelCount() * channelBytes;
    int length = buffer.size();
    const int numSamples = buffer.size() / sampleBytes;

    Q_ASSERT(length % sampleBytes == 0);
    Q_UNUSED(sampleBytes) // suppress warning in release builds

    unsigned char *ptr = reinterpret_cast<unsigned char *>(buffer.data());

    qreal phase = 0.0;

    const qreal d = 2 * M_PI / format.sampleRate();

    // We can't generate a zero-frequency sine wave
    const qreal startFreq = 1.0;
    const qreal endFreq = 22050;
    const qreal amplitude = 0.8;

    // Amount by which phase increases on each sample
    qreal phaseStep = d * startFreq;

    // Amount by which phaseStep increases on each sample
    // If this is non-zero, the output is a frequency-swept tone
    const qreal phaseStepStep = d * (endFreq - startFreq) / numSamples;

    while (length) {
        const qreal x = amplitude * qSin(phase);
        const qint16 value = realToPcm(x);
        for (int i=0; i<format.channelCount(); ++i) {
            qToLittleEndian<qint16>(value, ptr);
            ptr += channelBytes;
            length -= channelBytes;
        }

        phase += phaseStep;
        while (phase > 2 * M_PI)
            phase -= 2 * M_PI;
        phaseStep += phaseStepStep;
    }
}


ToneGenerator::ToneGenerator(const QAudioFormat &format,
                     qint64 durationUs,
                     int sampleRate,
                     QObject *parent)
    :   QIODevice(parent)
    ,   m_pos(0)
{
    if (format.isValid())
    {
        generateData(format, durationUs, sampleRate);
    }
    else
    {
        qWarning() << "Format invalid!";
    }
}

void ToneGenerator::start()
{
    open(QIODevice::ReadOnly);
}

void ToneGenerator::stop()
{
    m_pos = 0;
    close();
}

void ToneGenerator::generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate)
{
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channelCount() * channelBytes;

    qint64 length = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
                        * durationUs / 100000;

    Q_ASSERT(length % sampleBytes == 0);
    Q_UNUSED(sampleBytes) // suppress warning in release builds

    m_buffer.resize(length);


    if (format.byteOrder() == QAudioFormat::LittleEndian &&
        format.codec() == "audio/pcm" &&
        format.sampleType() == QAudioFormat::SignedInt &&
        format.sampleSize() == 16)
    {
        generateSweep(format, m_buffer);
        return;
    }

    unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());
    int sampleIndex = 0;

    while (length) {

        const qreal x = qSin(m_phase);
        m_phase += 440 * M_PI / format.sampleRate();
        while (m_phase >= 2 * M_PI)
        {
            m_phase -= 2 * M_PI;
        }


        for (int i=0; i<format.channelCount(); ++i) {
            if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
                const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
                const qint8 value = static_cast<qint8>(x * 127);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
                quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<quint16>(value, ptr);
                else
                    qToBigEndian<quint16>(value, ptr);
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
                qint16 value = static_cast<qint16>(x * 32767);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<qint16>(value, ptr);
                else
                    qToBigEndian<qint16>(value, ptr);
            }
            else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<float>(x, ptr);
                else
                    qToBigEndian<float>(x, ptr);
            }

            ptr += channelBytes;
            length -= channelBytes;
        }
        ++sampleIndex;
    }
}

qint64 ToneGenerator::readData(char *data, qint64 len)
{
    qint64 total = 0;
    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk);
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
        }
    }
    return total;
}

qint64 ToneGenerator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 ToneGenerator::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}
