#pragma once

#include <QIODevice>
#include <QAudioOutput>
#include <QAudioFormat>


class ToneGenerator : public QIODevice
{
    Q_OBJECT

public:
    ToneGenerator(const QAudioFormat &format, qint64 durationUs, int sampleRate, QObject *parent = nullptr);
    ~ToneGenerator() = default;

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

private:
    void generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate);

private:

    qreal m_phase = 0.0;
    qint64 m_pos = 0;
    QByteArray m_buffer;
};
