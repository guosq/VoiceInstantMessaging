#ifndef AUDIOIODEVICE_H
#define AUDIOIODEVICE_H

#include <QIODevice>
#include "AudioEncoder.h"

class AudioIODevice : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioIODevice(QObject *parent = nullptr);
    virtual ~AudioIODevice();

    void init(int sampleRate, int channelCnt, int sampleSize, QAudioFormat::Endian byteOrder, QAudioFormat::SampleType sampleType);
    void initDestination(const QString& hostAddr, int hostPort);
protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    void initEncoder();
private:

    AudioEncoder*   mpAudioEncoder;
};

#endif // AUDIOIODEVICE_H
