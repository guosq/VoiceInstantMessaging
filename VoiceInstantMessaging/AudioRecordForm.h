#ifndef AUDIORECORDFORM_H
#define AUDIORECORDFORM_H

#include <QWidget>
#include <QAudioInput>
#include "AudioIODevice.h"

namespace Ui {
class AudioRecordForm;
}

class AudioRecordForm : public QWidget
{
    Q_OBJECT

public:
    explicit AudioRecordForm(QWidget *parent = nullptr);
    ~AudioRecordForm();

private slots:
    void on_btnRecord_clicked();
    void capturePcmThread();
private:
    void startRecord();
    void stopRecord();
private:
    Ui::AudioRecordForm *ui;

    bool    mIsRecording = false;
    QAudioInput*    mpAudioInput = nullptr;
    AudioIODevice*  mpAudioIODevice = nullptr;
    QIODevice*      mpIODevice = nullptr;
//    QAudioFormat    mAudioFormat;

    QList<QAudioDeviceInfo> mAudioDeviceList;

    AudioEncoder*   mpAudioEncoder = NULL;

    int mReadOnceSize = 1024; // 每次从音频设备中读取的数据大小，
    int mPcmOffset = 0;
    int mPcmSize = 1024 * 2 * 2;
    char *mpPcm = nullptr; // 将要送入编码器的pcm裸数据
};

#endif // AUDIORECORDFORM_H
