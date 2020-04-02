#include <alsa/asoundlib.h>
#include <functional>
#include <iostream>

class Tamago
{
  public:
    // 録音設定
    unsigned int bufferTime = 1000; // バッファのサイズ ミリ秒

  private:
    // デバイス設定
    unsigned int channelCount = 8;                    // 録音ch数
    unsigned int samplingRate = 16000;                // サンプルレート
    snd_pcm_format_t format = SND_PCM_FORMAT_S24_3LE; // フォーマット

    // alsa
    snd_pcm_t *capture_handle;      //キャプチャデバイス
    snd_pcm_hw_params_t *hw_params; // キャプチャデバイスの設定
    char *buffer = nullptr;         // バッファ

    // misc
    unsigned int bufferReadCount = 0; // 今までにバッファを読んだ数
    unsigned int bufferSize = 0;

  public:
    // デバイス名, バッファミリ秒
    Tamago(const char *deviceName, unsigned int bufferT)
        : bufferTime(bufferT)
    {
        hwInit(deviceName);
        bufferSize = bufferTime * (samplingRate / 1000) * snd_pcm_format_width(format) / 8 * channelCount;
        buffer = new char[bufferSize];
    }

    // pcm デバイスの初期化
    void hwInit(const char *dev)
    {
        if (snd_pcm_open(&capture_handle, dev, SND_PCM_STREAM_CAPTURE, 0) < 0)
            throw std::runtime_error("cannot open audio device");

        if (snd_pcm_hw_params_malloc(&hw_params) < 0)
            throw std::runtime_error("cannot allocate hardware parameter structure");

        if (snd_pcm_hw_params_any(capture_handle, hw_params) < 0)
            throw std::runtime_error("cannot initialize hardware parameter structure");

        if (snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
            throw std::runtime_error("cannot set access type");

        if (snd_pcm_hw_params_set_format(capture_handle, hw_params, format) < 0)
            throw std::runtime_error("cannot set sample format");

        if (snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &samplingRate, 0) < 0)
            throw std::runtime_error("cannot set sample rate");

        if (snd_pcm_hw_params_set_channels(capture_handle, hw_params, channelCount) < 0)
            throw std::runtime_error("cannot set channel count");

        if (snd_pcm_hw_params(capture_handle, hw_params) < 0)
            throw std::runtime_error("cannot set parameters");

        snd_pcm_hw_params_free(hw_params);

        if (snd_pcm_prepare(capture_handle) < 0)
            throw std::runtime_error("cannot prepare audio interface for use");
    }

    // お片付け
    ~Tamago()
    {
        snd_pcm_close(capture_handle);
    }

    // バッファがたまるたびにコールバックが呼ばれる
    // コールバック関数がfalseを返すと終了
    void getBuffer(const std::function<bool(unsigned int, unsigned int, char *, unsigned int)> &callback)
    {
        for (;;) {
            int readBuffFrames = snd_pcm_readi(capture_handle, buffer, bufferTime * (samplingRate / 1000));
            unsigned int timeStart = bufferTime * bufferReadCount;
            unsigned int timeEnd = bufferTime * (bufferReadCount + 1);
            if (!callback(timeStart, timeEnd, buffer, readBuffFrames * snd_pcm_format_width(format) / 8 * channelCount))
                break;
            bufferReadCount++;
        }
    }
};