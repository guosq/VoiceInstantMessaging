## 局域网实时语音 (Voice instant messaging in LAN)

基于Qt+FFMpeg的局域网实时语音，语音数据为AAC。
跨平台：win ubuntu android

## 基本原理

1. 采用QtMultiMedia实时采集、播放PCM数据
2. 使用FFMpeg将PCM数据编码为AAC
3. 使用FFMpeg将AAC数据解码为PCM
4. UDP传输