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

#include <fstream>
#include <iostream>

class RIFF
{
  private:
    std::ofstream of;
    int riffStartPos = 0;
    int dataStartPos = 0;

  public:
    // ヘッダを作る
    RIFF(const char *fname, unsigned int channelCount, unsigned int bitSize, unsigned long int samplingRate)
    {
        of.open(fname, std::ios::out | std::ios::binary | std::ios::trunc);                                // ファイルを開く
        of.seekp(0, std::ios::beg);                                                                        // ファイルポインタをファイル先頭に
        of.write("RIFF", 4);                                                                               // RIFFヘッダ
        riffStartPos = of.tellp();                                                                         // RIFFヘッダ終了位置を保存しておく
        of.seekp(sizeof(char) * 4, std::ios::cur);                                                         // 終了までわからないチャンクサイズ書き込み用スペースを確保
        of.write("WAVE", 4);                                                                               // WAVEヘッダ
        of.write("fmt ", 4);                                                                               // fmt ヘッダ
        of.write(reinterpret_cast<const char *>(new int(16)), 4);                                          // fmt ヘッダ長
        of.write(reinterpret_cast<const char *>(new int(1)), 2);                                           // LPCM
        of.write(reinterpret_cast<const char *>(&channelCount), 2);                                        // チャネル数
        of.write(reinterpret_cast<const char *>(&samplingRate), 4);                                        // サンプリングレート
        of.write(reinterpret_cast<const char *>(new int((bitSize / 8) * samplingRate * channelCount)), 4); // データレート (量子化ビット数/8) *  チャンネル数 * サンプリングレート
        of.write(reinterpret_cast<const char *>(new int((bitSize / 8) * channelCount)), 2);                // ブロックサイズ 量子化ビット数 / 8 * チャンネル数
        of.write(reinterpret_cast<const char *>(&bitSize), 2);                                             // サンプルあたりのビット数
        of.write("data", 4);                                                                               // dataヘッダ
        dataStartPos = of.tellp();                                                                         // data開始位置を保存しておく
        of.seekp(sizeof(char) * 4, std::ios::cur);                                                         // 終了までわからないサイズ書き込み用スペースを開ける
    }

    void write(char *out, size_t size)
    {
        of.write(out, size);
        uint32_t fileSize = of.tellp();                                                    // fileSize
        of.seekp(riffStartPos, std::ios::beg);                                             // チャンクサイズ書き込み位置に移動
        of.write(reinterpret_cast<const char *>(new int(fileSize - 8)), 4);                // チャンクサイズ 総データ数 - RIFFヘッダ(4byte)とRIFFサイズ(4byte)
        of.seekp(dataStartPos, std::ios::beg);                                             // サブチャンクサイズ書き込み位置に移動
        of.write(reinterpret_cast<const char *>(new int(fileSize - dataStartPos - 4)), 4); // サブチャンクサイズ 総データ数 - RIFFヘッダ＋ RIFFサイズ + WAVヘッダ + チャンクヘッダ(4byte)
        of.seekp(fileSize, std::ios::beg);                                                 // 元の位置に戻る
    }

    // ファイルを閉じる前に、ファイルサイズを保存してお片付け
    ~RIFF()
    {
        of.close();
    }
};
