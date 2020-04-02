#include <fstream>
#include <iostream>

// http://chichimotsu.hateblo.jp/entry/20110812/1313173801

class RIFF
{
  private:
    std::ofstream of;
    int riff_p = 0;
    int data_p = 0;

  public:
    // ヘッダを作る
    RIFF(const char *fname, unsigned int channelCount, unsigned int bitSize, unsigned long int samplingRate)
    {
        int buf = 0;
        short buf2 = 0;
        of.open(fname, std::ios::out | std::ios::binary | std::ios::trunc);
        of.seekp(0, std::ios::beg); // ファイルポインタを頭へ

        of.write("RIFF", 4); // RIFFヘッダ
        riff_p = of.tellp();
        of.seekp(sizeof(char) * 4, std::ios::cur); // サイズを出力するための場所を空ける
        of.write("WAVE", 4);                       // WAVEヘッダ
        of.write("fmt ", 4);                       // fmt ヘッダ
        buf = 16;
        of.write((char *)&buf, 4); // fmt ヘッダ長
        buf2 = 1;
        of.write((char *)&buf2, 2); // リニアPCM

        buf2 = channelCount;
        of.write((char *)&buf2, 2); // チャネル数

        buf = samplingRate;
        of.write((char *)&buf, 4); // サンプリングレート

        buf = samplingRate * channelCount;
        of.write((char *)&buf, 4); // データ速度 チャンネル数 * サンプリングレート

        buf2 = (bitSize / 2) * channelCount;
        of.write((char *)&buf2, 2); // ブロックサイズ 16bit (2byte) * チャンネル数

        buf2 = bitSize;
        of.write((char *)&buf2, 2); // サンプルあたりのビット数

        of.write("data", 4); // dataヘッダ
        data_p = of.tellp();
        of.seekp(sizeof(char) * 4, std::ios::cur); // 4バイト分進める
    }

    void write(char *out, size_t size)
    {
        of.write(out, size);
    }

    // ファイルを閉じる前に、ファイルサイズを保存してお片付け
    ~RIFF()
    {
        int buf = 0;
        int last_p = of.tellp();
        of.seekp(riff_p, std::ios::beg);
        buf = last_p - 8;
        of.write((char *)&buf, 4);
        of.seekp(data_p, std::ios::beg);
        buf = last_p - data_p - 4;
        of.write((char *)&buf, 4);

        of.close();
    }
};
