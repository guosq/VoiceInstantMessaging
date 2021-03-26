#include "AudioIODevice.h"
#include <QDateTime>

AudioIODevice::AudioIODevice(QObject *parent) :
    QIODevice(parent),
    mpAudioEncoder(nullptr)
{
    mpAudioEncoder = new AudioEncoder(this);
}

AudioIODevice::~AudioIODevice()
{
}

void AudioIODevice::init(int sampleRate, int channelCnt, int sampleSize, QAudioFormat::Endian byteOrder, QAudioFormat::SampleType sampleType)
{
    if (mpAudioEncoder) {
        mpAudioEncoder->init(sampleRate, channelCnt, sampleSize, byteOrder, sampleType);
    }
}

void AudioIODevice::initDestination(const QString &hostAddr, int hostPort)
{    
    if (mpAudioEncoder) {
        mpAudioEncoder->initDestination(hostAddr, hostPort);
    }
}

qint64 AudioIODevice::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return -1;
}
// 音频设备按25Hz吐数据
qint64 AudioIODevice::writeData(const char *data, qint64 len)
{
    qDebug() << "AudioIODevice::writeData " << len << QDateTime::currentMSecsSinceEpoch();
    if (mpAudioEncoder) {
        mpAudioEncoder->processAudioPcmData(data, len);
    }
    return len;
}
