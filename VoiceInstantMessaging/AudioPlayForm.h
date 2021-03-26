#ifndef AUDIOPLAYFORM_H
#define AUDIOPLAYFORM_H

#include <QWidget>
#include <QIODevice>
#include <QAudioOutput>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "AudioDecoder.h"

namespace Ui {
class AudioPlayForm;
}

class AudioPlayForm : public QWidget
{
    Q_OBJECT

public:
    explicit AudioPlayForm(QWidget *parent = nullptr);
    ~AudioPlayForm();

private slots:
    void on_btnStart_clicked();

    void on_btnClear_clicked();
    void onReceivedAudioData();
private:
    bool startPlay();
    void stopPlay();
    void playAudioData(const QNetworkDatagram& datagram);
private:
    Ui::AudioPlayForm *ui;
    QIODevice*        mpAudioIODevice;
    QAudioOutput*     mpAudioOutput;
    QUdpSocket*       mpUdpSocket;
    QList<QAudioDeviceInfo> mAudioDeviceList;
    bool              mIsPlaying = false;

    AudioDecoder*     mpDecoder = nullptr;

};

#endif // AUDIOPLAYFORM_H
