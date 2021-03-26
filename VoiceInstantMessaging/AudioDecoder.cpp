#include "AudioDecoder.h"
#include <QDebug>
#include <QThread>

AudioDecoder::AudioDecoder(QObject *parent)
    : QObject(parent)
{

}

AudioDecoder::~AudioDecoder()
{
    if (mIsInited) {
        avcodec_free_context(&c);
        av_parser_close(parser);
        av_frame_free(&decoded_frame);
        av_packet_free(&pkt);
    }
}

void AudioDecoder::initAudioParam(int sampleRate, int channelCout)
{
    mSampleRate = sampleRate;
    mChannelCount = channelCout;
    init();
}

bool AudioDecoder::decodeAAC(const uint8_t *pAACData, int size)
{
    if (!mIsInited) return false;
    int ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                               pAACData, size,
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    if (ret < 0) {
        qDebug() << "AudioDecoder::decodeAAC parse failed: " << stderr;
        return false;
    }
    if (pkt->size) {
        decode(c, pkt, decoded_frame);
    }
    return true;
}

bool AudioDecoder::decodeAAC(const QByteArray &aacData)
{
    const uint8_t* pAACData = (const uint8_t*)aacData.data();
    return decodeAAC(pAACData, aacData.size());
}

void AudioDecoder::init()
{
    mIsInited = initDecoder();
    if (!mIsInited) return;
    mIsInited = initPlayer();
}

bool AudioDecoder::initDecoder()
{
    pkt = av_packet_alloc();
    // find decoder;
    codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (!codec) {
        qDebug() << "AudioDecoder::init failed: aac decoder not found";
        return false;
    }
    parser = av_parser_init(codec->id);
    if (!parser) {
        qDebug() << "AudioDecoder::init failed: aac parser init failed";
        return false;
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        qDebug() << "AudioDecoder::init failed: aac codec context allock failed";
        return false;
    }
    c->codec_type = AVMEDIA_TYPE_AUDIO;
    c->sample_rate = mSampleRate;
    c->channels = mChannelCount;
    c->channel_layout = av_get_default_channel_layout(mChannelCount);

    if (avcodec_open2(c, codec, NULL) < 0) {
        qDebug() << "AudioDecoder::init failed: aac codec open failed";
        return false;
    }

    decoded_frame = av_frame_alloc();
    if (!decoded_frame) {
        qDebug() << "AudioDecoder::init failed: decoded frame alloc failed";
        return false;
    }
    // 初始化重采样上下文
    aResampleCtx = swr_alloc_set_opts(aResampleCtx,
                                      av_get_default_channel_layout(mChannelCount),
                                      aFormat_out,
                                      mSampleRate,
                                      av_get_default_channel_layout(mChannelCount),
                                      aFormat_in,
                                      mSampleRate,
                                      0,
                                      NULL);
    swr_init(aResampleCtx);

    qDebug() << "AudioDecoder::initDecoder " << c->sample_fmt << c->sample_rate << c->channels;
    return true;
}

bool AudioDecoder::initPlayer()
{
    QAudioFormat format;
    format.setSampleRate(mSampleRate);
    format.setChannelCount(mChannelCount);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    QAudioFormat preferFormat = info.preferredFormat();
    qDebug() << "prefer format: " << preferFormat.sampleRate()
             << preferFormat.channelCount()
             << preferFormat.sampleSize()
             << preferFormat.codec()
             << preferFormat.byteOrder()
             << preferFormat.sampleType();
    if (!info.isFormatSupported(format)) {
        qDebug() << "defalut output device not support format";
        return false;
    }

    mpAudioOutput = new QAudioOutput(format, this);
//    mpAudioOutput->setBufferSize(8192);

    mpPlayDevice = mpAudioOutput->start();

    return true;
}

bool AudioDecoder::decode(AVCodecContext *pCodecContext, AVPacket *packet, AVFrame *frame)
{
    int ret = avcodec_send_packet(pCodecContext, packet);
    if (ret < 0) {
        qDebug() << "AudioDecoder::decode send packet failed";
        return false;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(pCodecContext, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return false;
        } else if (ret < 0) {
            qDebug() << "AudioDecoder::decode decoding failed";
            return false;
        }

        uint8_t *data[2] = {0};
        int nb_samples = av_rescale_rnd(swr_get_delay(aResampleCtx, mSampleRate) + frame->nb_samples,
                                        mSampleRate,
                                        pCodecContext->sample_rate,
                                        AV_ROUND_UP);
        if (aResampleFrame) {
            if (aResampleFrame->nb_samples != nb_samples) {
                av_frame_free(&aResampleFrame);
                aResampleFrame = nullptr;
            }
        }
        if (!aResampleFrame) {
            aResampleFrame = av_frame_alloc();
            aResampleFrame->format = aFormat_out;
            aResampleFrame->channel_layout = av_get_default_channel_layout(mChannelCount);
            aResampleFrame->sample_rate = mSampleRate;
            aResampleFrame->nb_samples = nb_samples;
        }
        int byteCnt = aResampleFrame->nb_samples * mChannelCount * 2;
        unsigned char* pcm = new uint8_t[byteCnt];
        data[0] = pcm;

        int ret = swr_convert(aResampleCtx,
                              data,
                              aResampleFrame->nb_samples,
                              (const uint8_t**)frame->data,
                              frame->nb_samples);

        if (ret >= 0) {
            int size = ret * frame->channels * av_get_bytes_per_sample((AVSampleFormat)aFormat_out);
//            while (mpAudioOutput->bytesFree() < size) {
//                QThread::msleep(5);
//            }
            if (mpPlayDevice) {
                mpPlayDevice->write((const char*)pcm, byteCnt);
            }
        }
        delete [] pcm;

        // play audio        
//        playAudio(frame);
    }
    return true;
}

void AudioDecoder::playAudio(AVFrame *frame)
{
    if (mpPlayDevice) {
        int pcmSize = av_get_bytes_per_sample(aFormat_out) * mSampleRate;
        mpPlayDevice->write((const char*)frame->data, pcmSize);
    }
}
