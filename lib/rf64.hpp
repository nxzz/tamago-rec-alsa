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

class RF64
{
  private:
    std::ofstream of;
    std::streampos rf64SizePos = 0;
    std::streampos dataSizePos = 0;

  public:
    // ヘッダを作る
    RF64(const char *fname, unsigned int channelCount, unsigned int bitSize, unsigned long int samplingRate)
    {
        // wave 4byte + 4byte + 4byte = 12byte
        of.open(fname, std::ios::out | std::ios::binary | std::ios::trunc); // ファイルを開く
        of.seekp(0, std::ios::beg);                                         // ファイルポインタをファイル先頭に
        of.write("RF64", 4);                                                // RF64ヘッダ
        of.write(reinterpret_cast<const char *>(new int(-1)), 4);           // RF64では利用しない -1
        of.write("WAVE", 4);                                                // WAVEヘッダ

        //ds64 4byte + 4byte + 28byte = 36byte
        of.write("ds64", 4);                                           // ds64 チャンクヘッダ
        of.write(reinterpret_cast<const char *>(new uint32_t(28)), 4); // ds64 チャンクヘッダ長
        rf64SizePos = of.tellp();                                      // RF64 ブロックサイズ予定地
        of.seekp(sizeof(char) * 8, std::ios::cur);                     // 8byte skip
        dataSizePos = of.tellp();                                      // data ブロックサイズ予定地
        of.seekp(sizeof(char) * 8, std::ios::cur);                     // 8byte skip
        // sampleCountPos = of.tellp();                                   // サンプル数予定地 非圧縮なPCMの場合 factチャンクがないから使わない？
        of.seekp(sizeof(char) * 8, std::ios::cur);                    // 8byte skip
        of.write(reinterpret_cast<const char *>(new uint32_t(0)), 4); // Table count

        // fmt 4 byte + 4 byte + 16 byte = 24byte
        of.write("fmt ", 4);                                                                               // fmt ヘッダ
        of.write(reinterpret_cast<const char *>(new int(16)), 4);                                          // fmt ヘッダ長
        of.write(reinterpret_cast<const char *>(new int(1)), 2);                                           // LPCM
        of.write(reinterpret_cast<const char *>(&channelCount), 2);                                        // チャネル数
        of.write(reinterpret_cast<const char *>(&samplingRate), 4);                                        // サンプリングレート
        of.write(reinterpret_cast<const char *>(new int((bitSize / 8) * samplingRate * channelCount)), 4); // データレート (量子化ビット数 / 8 ) * チャンネル数 * サンプリングレート
        of.write(reinterpret_cast<const char *>(new int((bitSize / 8) * channelCount)), 2);                // ブロックサイズ (量子化ビット数 / 8) * チャンネル数
        of.write(reinterpret_cast<const char *>(&bitSize), 2);                                             // サンプルあたりのビット数

        // data 4byte + 4byte + nbyte
        of.write("data", 4);                                      // dataヘッダ
        of.write(reinterpret_cast<const char *>(new int(-1)), 4); // RF64では利用しない -1
    }

    void write(char *out, size_t size)
    {
        of.write(out, size);
        uint64_t fileSize = of.tellp(); // ファイルサイズ取得

        of.seekp(rf64SizePos, std::ios::beg);                                                     // ファイルサイズ書き込み位置に移動
        of.write(reinterpret_cast<const char *>(new uint32_t(fileSize & 0xFFFFFFFF)), 4);         // ファイルサイズ下位4byte
        of.write(reinterpret_cast<const char *>(new uint32_t((fileSize >> 32) & 0xFFFFFFFF)), 4); // ファイルサイズ上位4byte

        of.seekp(dataSizePos, std::ios::beg);                                                     // data ブロックサイズ書き込み位置に移動
        uint64_t dataSize = fileSize - 12 - 36 - 24 - 8;                                          // data ブロックサイズ
        of.write(reinterpret_cast<const char *>(new uint32_t(dataSize & 0xFFFFFFFF)), 4);         // data ブロックサイズ下位4byte ファイルサイズ - RIFF- DS64 - fmt - dataヘッダ
        of.write(reinterpret_cast<const char *>(new uint32_t((dataSize >> 32) & 0xFFFFFFFF)), 4); // data ブロックサイズ上位4byte

        of.seekp(fileSize, std::ios::beg); // 元の位置に戻る
    }

    // ファイルを閉じる前に、ファイルサイズを保存してお片付け
    ~RF64()
    {
        of.close();
    }
};
