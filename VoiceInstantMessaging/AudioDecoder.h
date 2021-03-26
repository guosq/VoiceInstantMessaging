#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QObject>

#include <QAudioOutput>
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

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

class AudioDecoder : public QObject
{
    Q_OBJECT
public:
    explicit AudioDecoder(QObject *parent = nullptr);
    virtual ~AudioDecoder();
    void initAudioParam(int sampleRate, int channelCout);

    bool decodeAAC(const uint8_t *pAACData, int size);
    bool decodeAAC(const QByteArray& aacData);
signals:

private:
    void init();
    bool initDecoder();
    bool initPlayer();
    bool decode(AVCodecContext* pCodecContext, AVPacket* packet, AVFrame* frame);
    void playAudio(AVFrame* frame);
private:
    bool    mIsInited = false;

    const AVCodec *codec;
    AVCodecContext *c= NULL;
    AVCodecParserContext *parser = NULL;
    int len, ret;
    FILE *f, *outfile;
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data;
    size_t   data_size;
    AVPacket *pkt;
    AVFrame *decoded_frame = NULL;
    enum AVSampleFormat sfmt;
    int n_channels = 0;
    const char *fmt;

    int mSampleRate;
    int mChannelCount;

    SwrContext*         aResampleCtx = NULL;
    AVFrame*            aResampleFrame = NULL;

//    int                 aSampleRate_out = 16000; // 音频输出采样率
    enum AVSampleFormat aFormat_out = AV_SAMPLE_FMT_S16;
//    int                 aChannelCount_out = 1;

//    int                 aSampleRate_in = 16000; // 音频输入采样率
    enum AVSampleFormat aFormat_in = AV_SAMPLE_FMT_FLTP;
//    int                 aChannelCount_in = 1;

    QAudioOutput*       mpAudioOutput = nullptr;
    QIODevice*          mpPlayDevice = nullptr;
};

#endif // AUDIODECODER_H
