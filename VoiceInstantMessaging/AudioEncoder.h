#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include <QObject>
#include <QAudioFormat>
#include <QUdpSocket>
#include <QFile>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/time.h>
    #include <libavdevice/avdevice.h>
    #include <libswscale/swscale.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
    #include <libavutil/audio_fifo.h>
    #include <libavfilter/avfilter.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/hwcontext.h>
}

class AudioEncoder : public QObject
{
    Q_OBJECT
public:
    explicit AudioEncoder(QObject *parent = nullptr);
    virtual ~AudioEncoder();

    void init(int sampleRate, int channelCnt, int sampleSize, QAudioFormat::Endian byteOrder, QAudioFormat::SampleType sampleType);
    void initDestination(const QString& hostAddr, int hostPort);

    void processAudioPcmData(const char* pData, qint64 size);

    void terminateEncoder();
signals:

private:
    bool initEncoder();
    bool checkSampleFmt(const AVCodec* pCodec, enum AVSampleFormat sampleFmt);
    int flushEncoder(AVFormatContext* fmt_ctx, unsigned int stream_index);
    QByteArray packageASTS(const char* pAAC, int size);
    int frequencyIndex(int freq);
    enum AVSampleFormat inputSampleFmt(int sampleSize, int sampleType);
private:
    int                         mSampleRate;
    int                         mChannelCnt;
    int                         mSampleSize;
    QAudioFormat::Endian        mByteOrder;
    QAudioFormat::SampleType    mSampleType;

    QUdpSocket                  mUdpSocket;
    QHostAddress                mHostAddress;
    QString                     mHostAddr;
    int                         mHostPort;

    bool                        mIsCanEncode = false;

    AVCodecContext*             mpCodecCtx = nullptr;//编码上下文
    AVCodec*                    mpCodec = nullptr;//编码器
    AVFrame*                    mpFrame = nullptr;// 编码前的音频数据
    AVPacket*                   mpPkt = nullptr;//编码后的音频数据
    int                         mSize;
    char*                       mpFrameBuff = nullptr;

    SwrContext*                 mpSwr = nullptr;
    uint8_t*                    mpOuts[2];
    int                         mLen = 4096;

    QFile                       mOutFile;

    uint8_t                     mADTSBuffer[1024 * 10] = {0};
};

#endif // AUDIOENCODER_H
