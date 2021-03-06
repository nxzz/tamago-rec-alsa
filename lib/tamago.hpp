// Copyright [2020] [Rimpei Kunimoto]

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

// 	http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
    void getBuffer(const std::function<bool(unsigned int, unsigned long long int, char *, int)> &callback)
    {
        unsigned int bufferReadCount = 0;    // 今までにバッファを読んだ回数
        unsigned long long int readByte = 0; // 今までバッファから読み込んだバイト数

        for (;;) {
            // たまごから読みだす
            int readBuffFrames = snd_pcm_readi(capture_handle, buffer, bufferTime * (samplingRate / 1000));

            // 読み出しがこけたら、リカバリを試す
            if (readBuffFrames < 0) {
                snd_pcm_recover(capture_handle, readBuffFrames, 1);          //サイレント修正
                callback(bufferReadCount, readByte, buffer, readBuffFrames); // エラーの存在を通知するために、コールバックは呼ぶ
                continue;
            }
            readByte += readBuffFrames * snd_pcm_format_width(format) / 8 * channelCount;
            bufferReadCount++;

            if (!callback(bufferReadCount, readByte, buffer, readBuffFrames * snd_pcm_format_width(format) / 8 * channelCount))
                break;
        }
    }
};