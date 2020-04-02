#include "./riff.hpp"
#include "./tamago.hpp"
#include <fstream>
#include <functional>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    // 1秒ごとに取り込む
    Tamago egg(argv[1], 1000);

    // ./rec.wav に 8ch 16khz/24bit で書き込む
    RIFF wav("./rec.wav", 8, 24, 16000);

    egg.getBuffer([&](unsigned int timeStart, unsigned int timeEnd, char *buffer, unsigned int bufferSize) {
        cout << timeEnd << endl;
        wav.write(buffer, bufferSize);
        if (timeEnd >= 10000)
            return false;
        return true;
    });

    return 1;
}