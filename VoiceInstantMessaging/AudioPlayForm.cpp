#include "AudioPlayForm.h"
#include "ui_AudioPlayForm.h"
#include <QDateTime>

AudioPlayForm::AudioPlayForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioPlayForm)
    , mpAudioIODevice(NULL)
    , mpAudioOutput(NULL)
    , mpUdpSocket(NULL)
{
    ui->setupUi(this);
    // 该功能未调通，暂时使用默认播放设备
    ui->cmbOutputDevices->setVisible(false);
    ui->labOutputDevice->setVisible(false);
    mAudioDeviceList = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    for (auto deviceInfo: mAudioDeviceList) {
        ui->cmbOutputDevices->addItem(deviceInfo.deviceName());
        qDebug() << deviceInfo.deviceName();
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

    // 初始化解码器
    mpDecoder = new AudioDecoder(this);

    mpUdpSocket = new QUdpSocket(this);
}

AudioPlayForm::~AudioPlayForm()
{
    mpUdpSocket->deleteLater();
    mpUdpSocket = nullptr;
    delete ui;
}

void AudioPlayForm::on_btnStart_clicked()
{
    if (mIsPlaying) {
        stopPlay();
        mIsPlaying = false;
        ui->btnStart->setText("Start");
    } else {
        if (startPlay()) {
            mIsPlaying = true;
            ui->btnStart->setText("Stop");
        }
    }
}

void AudioPlayForm::on_btnClear_clicked()
{
    ui->textEdit->clear();
}

void AudioPlayForm::onReceivedAudioData()
{
    while (mpUdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = mpUdpSocket->receiveDatagram();
        playAudioData(datagram);
    }
}

bool AudioPlayForm::startPlay()
{
//    if (mAudioDeviceList.size() < 1) {
//        qDebug() << "MainWindow::startPlay failed no available output device";
//        return false;
//    }

//    QAudioFormat format;
//    format.setSampleRate(ui->cmbSampleRate->currentText().toInt());
//    format.setChannelCount(ui->cmbChCnt->currentText().toInt());
//    format.setSampleSize(ui->cmbSampleSize->currentText().toInt());
//    format.setCodec("audio/pcm");
//    format.setByteOrder(ui->cmbByteOrder->currentData().value<QAudioFormat::Endian>());
//    format.setSampleType(ui->cmbSampleType->currentData().value<QAudioFormat::SampleType>());

////    QAudioDeviceInfo deviceInfo = mAudioDeviceList.at(ui->cmbOutputDevices->currentIndex());
//    QAudioDeviceInfo deviceInfo = QAudioDeviceInfo::defaultOutputDevice();
//    QAudioFormat defaultFormat = deviceInfo.preferredFormat();
//    qDebug() << "default format: "
//             << defaultFormat.sampleRate()
//             << defaultFormat.sampleSize()
//             << defaultFormat.sampleType()
//             << defaultFormat.codec()
//             << defaultFormat.byteOrder()
//             << defaultFormat.channelCount();

//    if (!deviceInfo.isFormatSupported(format)) {
//        qDebug() << "default output device not support format";
//        return false;
//    }
//    qDebug() << "start play audio "
//             << ui->cmbSampleRate->currentText().toInt()
//             << ui->cmbChCnt->currentText().toInt()
//             << ui->cmbSampleSize->currentText().toInt();

//    mpAudioOutput = new QAudioOutput(format, this);
//    qDebug()<< "Audio output buffer " << mpAudioOutput->bufferSize();

//    mpAudioIODevice = mpAudioOutput->start();

    if (mpDecoder) {
        mpDecoder->initAudioParam(ui->cmbSampleRate->currentText().toInt(),
                                  ui->cmbChCnt->currentText().toInt());
    }
    mpUdpSocket->bind(QHostAddress::Any, ui->leLocalPort->text().toInt());
    connect(mpUdpSocket, &QUdpSocket::readyRead, this, &AudioPlayForm::onReceivedAudioData);

    return true;
}

void AudioPlayForm::stopPlay()
{
//    if (mpAudioOutput) {
//        mpAudioOutput->stop();
//        delete mpAudioOutput;
//        mpAudioOutput = NULL;
//    }

    disconnect(mpUdpSocket, &QUdpSocket::readyRead, this, &AudioPlayForm::onReceivedAudioData);
    mpUdpSocket->close();
}

void AudioPlayForm::playAudioData(const QNetworkDatagram &datagram)
{
    ui->textEdit->append(QString("%1:%2, %3 -- %4").arg(datagram.senderAddress().toString())
                                             .arg(datagram.senderPort())
                                             .arg(datagram.data().size())
                                             .arg(QDateTime::currentMSecsSinceEpoch()));
//    if (mpAudioIODevice) {
//        if (mpAudioOutput->bytesFree() >= datagram.data().size())
//            mpAudioIODevice->write(datagram.data());
//    }
    if (mpDecoder) {
        mpDecoder->decodeAAC(datagram.data());
    }
}
