#include "AudioRecordForm.h"
#include "ui_AudioRecordForm.h"
#include <QAudioDeviceInfo>
#include <QThread>
#include <QtConcurrent>


AudioRecordForm::AudioRecordForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioRecordForm),
    mIsRecording(false),
    mpAudioInput(NULL),
    mpAudioIODevice(NULL)
{
    ui->setupUi(this);

    mAudioDeviceList = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for (auto deviceInfo : mAudioDeviceList) {
        ui->cmbAudioDevices->addItem(deviceInfo.deviceName());
    }
    QStringList sampleRate;
    sampleRate << "8000"
               << "11025"
               << "16000"
               << "22050"
               << "32000"
               << "44100"
               << "48000"
               << "88200"
               << "96000";
    ui->cmbSampleRate->addItems(sampleRate);

    ui->cmbChCnt->addItem(QStringLiteral("1"), QVariant(1));
    ui->cmbChCnt->addItem(QStringLiteral("2"), QVariant(2));
    ui->cmbChCnt->addItem(QStringLiteral("4"), QVariant(4));

    ui->cmbSampleSize->addItem(QStringLiteral("8"), QVariant(8));
    ui->cmbSampleSize->addItem(QStringLiteral("16"), QVariant(16));
    ui->cmbSampleSize->addItem(QStringLiteral("32"), QVariant(32));

    ui->cmbSampleType->addItem(QStringLiteral("SignedInt"), QVariant(QAudioFormat::SignedInt));
    ui->cmbSampleType->addItem(QStringLiteral("UnSignedInt"), QVariant(QAudioFormat::UnSignedInt));
    ui->cmbSampleType->addItem(QStringLiteral("Float"), QVariant(QAudioFormat::Float));

    ui->cmbByteOrder->addItem(QStringLiteral("LittleEndian"), QVariant(QAudioFormat::LittleEndian));
    ui->cmbByteOrder->addItem(QStringLiteral("BigEndian"), QVariant(QAudioFormat::BigEndian));

}

AudioRecordForm::~AudioRecordForm()
{
    if (mIsRecording) {
        stopRecord();
    }
    delete ui;
}

void AudioRecordForm::on_btnRecord_clicked()
{
    if (!mIsRecording) {
        if (ui->cmbAudioDevices->count() < 1) {
            qDebug() << "start audio record failed: no avaliable audio input device";
            return;
        }
        mIsRecording = true;
        ui->btnRecord->setText("stop record");

        startRecord();

    } else {
        ui->btnRecord->setText("start record");
        stopRecord();
    }
}

void AudioRecordForm::startRecord()
{
    if (mAudioDeviceList.size() < 1) return;

    QAudioFormat audioFormat;
    audioFormat.setSampleRate(ui->cmbSampleRate->currentText().toInt());
    audioFormat.setChannelCount(ui->cmbChCnt->currentText().toInt());
    audioFormat.setSampleSize(ui->cmbSampleSize->currentText().toInt());
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(ui->cmbByteOrder->currentData().value<QAudioFormat::Endian>());
    audioFormat.setSampleType(ui->cmbSampleType->currentData().value<QAudioFormat::SampleType>());

    mpAudioInput = new QAudioInput(mAudioDeviceList.at(ui->cmbAudioDevices->currentIndex()), audioFormat, this);

    //方法1：等待音频设备吐数据
//    mpAudioIODevice = new AudioIODevice(this);
//    mpAudioIODevice->init(audioFormat.sampleRate(),
//                          audioFormat.channelCount(),
//                          audioFormat.sampleSize(),
//                          audioFormat.byteOrder(),
//                          audioFormat.sampleType());
//    mpAudioIODevice->initDestination(ui->leHostAddr->text(), ui->leHostPort->text().toInt());
//    mpAudioIODevice->open(QIODevice::WriteOnly);
//    mpAudioInput->start(mpAudioIODevice);

    //方法2： 主动从音频缓冲区中取数据
    mpIODevice = mpAudioInput->start();
    mPcmSize = 1024 * audioFormat.channelCount() * audioFormat.sampleSize() / 8;
    mpPcm = new char[mPcmSize];
    //开始采集音频
    QtConcurrent::run(this, &AudioRecordForm::capturePcmThread);
}

void AudioRecordForm::stopRecord()
{
    mIsRecording = false;

    if (mpAudioInput) {
        mpAudioInput->stop();
        delete mpAudioInput;
        mpAudioInput = nullptr;
    }
    if (mpAudioIODevice) {
        mpAudioIODevice->close();
        delete mpAudioIODevice;
        mpAudioIODevice = nullptr;
    }
}

void AudioRecordForm::capturePcmThread()
{
    // 初始化编码器
    QAudioFormat audioFormat = mpAudioInput->format();
    mpAudioEncoder = new AudioEncoder();
    mpAudioEncoder->init(audioFormat.sampleRate(),
                         audioFormat.channelCount(),
                         audioFormat.sampleSize(),
                         audioFormat.byteOrder(),
                         audioFormat.sampleType());
    mpAudioEncoder->initDestination(ui->leHostAddr->text(), ui->leHostPort->text().toInt());

    // 从缓冲区中读取pcm数据
    while (mIsRecording) {
        int remains = mPcmSize - mPcmOffset;//
        int ready = mpAudioInput->bytesReady();// 缓冲区中可以读取的数据大小
//        qDebug() << "AudioRecordForm::capturePcmThread " << ready << QDateTime::currentMSecsSinceEpoch();
        if (ready < mReadOnceSize) { // 可读取的数据太少，继续等待
            QThread::msleep(1);
            continue;
        }
        if (remains < mReadOnceSize) {//
            int len = mpIODevice->read(mpPcm + mPcmOffset, remains);
            mPcmOffset += len;
            if (mPcmOffset == mPcmSize && mpAudioEncoder) {
                mpAudioEncoder->processAudioPcmData(mpPcm, mPcmSize);
                mPcmOffset = 0;
            }
            continue;
        }
        // 从缓冲区中读取1024个字节，存入pcm中
        int len = mpIODevice->read(mpPcm + mPcmOffset, mReadOnceSize);
        mPcmOffset += len;

        // 已经读取的pcm长度足够一次编码使用，则将pcm数据送入编码器
        if (mPcmOffset == mPcmSize && mpAudioEncoder) {
            mpAudioEncoder->processAudioPcmData(mpPcm, mPcmSize);
            mPcmOffset = 0;
        }
    }

    if (mpAudioEncoder) {
        mpAudioEncoder->terminateEncoder();
        delete mpAudioEncoder;
        mpAudioEncoder = nullptr;
    }
}
