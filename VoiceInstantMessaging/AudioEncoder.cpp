#include "AudioEncoder.h"
#include <QDebug>
#include <QDateTime>
#include <QCoreApplication>

AudioEncoder::AudioEncoder(QObject *parent)
    : QObject(parent),
      mSampleRate(16000),
      mChannelCnt(1),
      mSampleSize(16),
      mByteOrder(QAudioFormat::LittleEndian),
      mSampleType(QAudioFormat::UnSignedInt)
{
}

AudioEncoder::~AudioEncoder()
{
    mUdpSocket.flush();
    mOutFile.flush();
    mOutFile.close();
}

void AudioEncoder::init(int sampleRate, int channelCnt, int sampleSize, QAudioFormat::Endian byteOrder, QAudioFormat::SampleType sampleType)
{
    mSampleRate = sampleRate;
    mChannelCnt = channelCnt;
    mSampleSize = sampleSize;
    mByteOrder  = byteOrder;
    mSampleType = sampleType;

    mOutFile.setFileName(QCoreApplication::applicationDirPath() + "/output.aac");
    mOutFile.open(QIODevice::WriteOnly);

    mIsCanEncode = initEncoder();
}

void AudioEncoder::initDestination(const QString &hostAddr, int hostPort)
{
    mHostAddr = hostAddr;
    mHostPort = hostPort;
    mHostAddress = QHostAddress(mHostAddr);
}

#define AAC_NB_SAMPLES 1024
void AudioEncoder::processAudioPcmData(const char *pData, qint64 size)
{
//    qDebug() << "AudioEncoder::processAudioPcmData raw data size " <<size << QDateTime::currentMSecsSinceEpoch();

    // 将PCM数据发送至指定地址
//    mUdpSocket.writeDatagram(QByteArray(pData, size), mHostAddress, mHostPort);

    if (mIsCanEncode) {

//        uint8_t* pFrameBuffer = (uint8_t*)mpFrame->data[0];
//        memcpy(pFrameBuffer, pData, size);

        // 重采样
//        int count = swr_convert(mpSwr, mpOuts, mLen * 4 , (const uint8_t**)&pFrameBuffer, mLen/4);
//        mpFrame->data[0] = (uint8_t*)mpOuts[0];
//        mpFrame->data[1] = (uint8_t*)mpOuts[1];

        // 重采样
        int count = swr_convert(mpSwr, mpFrame->data, mpFrame->nb_samples,
                                (const uint8_t**)&pData, mpFrame->nb_samples);

        int ret = avcodec_send_frame(mpCodecCtx, mpFrame);
        if (ret < 0) {
            qDebug() << "AudioEncoder::processAudioPcmData send raw audio data failed";
            char errStr[1024];
            av_strerror(ret, errStr, 1024);
            qDebug() << errStr;
            return;
        }
        while (ret >= 0) {
            ret = avcodec_receive_packet(mpCodecCtx, mpPkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                return;
            } else if (ret < 0) {
                char errStr[1024];
                av_strerror(ret, errStr, 1024);
                qDebug() << "AudioEncoder::processAudioPcmData encode audio data failed" << errStr;
                return;
            }

            // 添加ADTS头信息
            QByteArray adtsData = packageASTS((const char*)mpPkt->data, mpPkt->size);
            // 发送至指定地址
            mUdpSocket.writeDatagram(adtsData, mHostAddress, mHostPort);
            // 保存本地
//            mOutFile.write(adtsData);


            // 释放资源
            av_packet_unref(mpPkt);
        }
    }
    ////////////////////////////////////////////////
}

void AudioEncoder::terminateEncoder()
{
    if (mpCodecCtx) {
        avcodec_close(mpCodecCtx);
        avcodec_free_context(&mpCodecCtx);
    }
    if (mpSwr) {
        swr_free(&mpSwr);
    }
    if (mpFrame) {
        av_frame_free(&mpFrame);
    }
    if (mpPkt) {
        av_packet_free(&mpPkt);
    }
}

bool AudioEncoder::initEncoder()
{
    // 初始化编码器
    mpCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);

    // 初始化编码上下文
    mpCodecCtx = avcodec_alloc_context3(mpCodec);
    mpCodecCtx->codec_id = AV_CODEC_ID_AAC;
    mpCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    mpCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;// 当前ffmpeg版本4.3只支持FLTP格式
    mpCodecCtx->sample_rate = mSampleRate;
    mpCodecCtx->channel_layout = av_get_default_channel_layout(mChannelCnt);
//    mpCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    mpCodecCtx->channels = mChannelCnt;
    mpCodecCtx->profile = FF_PROFILE_AAC_LOW;

    if (!checkSampleFmt(mpCodec, mpCodecCtx->sample_fmt)) {
        qDebug() << "AudioEncoder::initEncoder " << "not support sample formate: " << (enum AVSampleFormat)mpCodecCtx->sample_fmt;
        return false;
    }

    qDebug() << "AudioEncoder::initEncoder" << mpCodecCtx->channel_layout << av_get_default_channel_layout(mChannelCnt) << inputSampleFmt(mSampleSize, mSampleType);

    // 打开编码器
    if (int ret = avcodec_open2(mpCodecCtx, mpCodec, NULL) < 0) {
        char errStr[1024];
        av_strerror(ret, errStr, 1024);
        qDebug() << "AudioEncoder::initEncoder open failed: " <<errStr;
        return false;
    }

    // 初始化编码前的音频帧
    mpFrame = av_frame_alloc();
    mpFrame->nb_samples = mpCodecCtx->frame_size;
    mpFrame->format = mpCodecCtx->sample_fmt;
    mpFrame->channel_layout = mpCodecCtx->channel_layout;
    av_frame_get_buffer(mpFrame, 0);// allocate data buffer
    av_frame_make_writable(mpFrame);

    mSize = av_samples_get_buffer_size(NULL, mChannelCnt, mpFrame->nb_samples, (enum AVSampleFormat)mpFrame->format, 1);
    mpFrameBuff = (char*)av_malloc(mSize);
    avcodec_fill_audio_frame(mpFrame, mpCodecCtx->channels, mpCodecCtx->sample_fmt, (const uint8_t*)mpFrameBuff, mSize, 1);

    qDebug() << " av_samples_get_buffer_size " << mSize << mpFrame->nb_samples;
    // 编码后的音频数据
    mpPkt = av_packet_alloc();

    // 重采样上下文初始化
    mpSwr = swr_alloc();
    av_opt_set_int(mpSwr, "in_channel_layout", av_get_default_channel_layout(mChannelCnt), 0);
    av_opt_set_int(mpSwr, "out_channel_layout", av_get_default_channel_layout(mChannelCnt), 0);
    av_opt_set_int(mpSwr, "in_sample_rate", mSampleRate, 0);
    av_opt_set_int(mpSwr, "out_sample_rate", mSampleRate, 0);
    av_opt_set_sample_fmt(mpSwr, "in_sample_fmt", inputSampleFmt(mSampleSize, mSampleType), 0);
    av_opt_set_sample_fmt(mpSwr, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
    swr_init(mpSwr);

    mpOuts[0] = (uint8_t*)malloc(mLen);
    mpOuts[1] = (uint8_t*)malloc(mLen);


    return true;
}

bool AudioEncoder::checkSampleFmt(const AVCodec *pCodec, AVSampleFormat sampleFmt)
{
    const enum AVSampleFormat *p = pCodec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sampleFmt) {
            return true;
        }
        p++;
    }
    return false;
}

QByteArray AudioEncoder::packageASTS(const char *pAAC, int size)
{
    int profile = 2; // AAC LC;
    int freqIdx = frequencyIndex(mSampleRate);
    int chanCfg = mChannelCnt; // 声道数量
//    qDebug() << "AudioEncoder::packageASTS " << freqIdx << chanCfg;

    int len = size + 7;

    mADTSBuffer[0] = (unsigned char)0xFF;
    mADTSBuffer[1] = (unsigned char)0xF1;
    mADTSBuffer[2] = (unsigned char)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    mADTSBuffer[3] = (unsigned char)(((chanCfg & 3) << 6) + (len >> 11));
    mADTSBuffer[4] = (unsigned char)((len & 0x7FF) >> 3);
    mADTSBuffer[5] = (unsigned char)(((len & 7) << 5) + 0x1F);
    mADTSBuffer[6] = (unsigned char)(0xFC);

    memcpy(&mADTSBuffer[7], pAAC, size);

    QByteArray retData((const char*)mADTSBuffer, len);

    return retData;
}

int AudioEncoder::frequencyIndex(int freq)
{
    switch (freq) {
    case 96000: return 0;
    case 88200: return 1;
    case 64000: return 2;
    case 48000: return 3;
    case 44100: return 4;
    case 32000: return 5;
    case 24000: return 6;
    case 22050: return 7;
    case 16000: return 8;
    case 12000: return 9;
    case 11025: return 10;
    case 8000: return 11;
    case 7350: return 12;
    default: return 8;
    }
}

AVSampleFormat AudioEncoder::inputSampleFmt(int sampleSize, int sampleType)
{
    if (sampleType == QAudioFormat::UnSignedInt) {
        if (sampleSize == 8) return AV_SAMPLE_FMT_U8;
    }

    if (sampleType == QAudioFormat::SignedInt) {
        if (sampleSize == 16) return AV_SAMPLE_FMT_S16;
        if (sampleSize == 32) return AV_SAMPLE_FMT_S32;
    }

    if (sampleType == QAudioFormat::Float) {
        return AV_SAMPLE_FMT_FLT;
    }
    // 默认平面格式，float类型
    return AV_SAMPLE_FMT_FLTP;
}
