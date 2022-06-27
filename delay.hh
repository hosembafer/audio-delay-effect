/**
 * @file delay.hh
 * @brief Audio Delay linear effect
 * @author Rafael Hovhannisyan <hovhannisian.rafael@gmail.com>
 * @version 0.1
 * @date 2022-06-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>
#include <string>
#include <sndfile.hh>
#include <math.h>
#include <algorithm>

// delayTime      | MS |  [0, 1000] | 500
// feedback       | % |   [0, 100] | 50
// dryWet         | % |   [0, 100] | 50
// log                    [true/false] | false

using namespace std;

bool inRange(int low, int high, int x)
{
    return ((x - low) <= (high - low));
}

void delay(string inFilePath, string outFilePath, int delayTime = 500, int feedback = 50, int dryWet = 50, bool log = false)
{
    if (!inRange(0, 1000, delayTime))
        throw invalid_argument("Delay Time must be between 0 and 1000");
    if (!inRange(0, 100, feedback))
        throw invalid_argument("Feedback must be between 0 and 100");
    if (!inRange(0, 100, dryWet))
        throw invalid_argument("Dry/Wet must be between 0 and 100");

    float delayTimeSec = (float)delayTime / 1000;
    float feedbackRatio = (float)feedback / 100;
    float dryRatio = 1.0f - ((float)dryWet / 100);
    float wetRatio = (float)dryWet / 100;

    SndfileHandle inFile(inFilePath);
    float sampleRate = (float)inFile.samplerate();
    float frames = (float)inFile.frames();
    int channels = inFile.channels();
    float duration = frames / sampleRate;

    if (delayTimeSec > duration)
        throw invalid_argument("Delay Time must not exceed duration of input");

    SndfileHandle outFile(outFilePath, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, channels, sampleRate);

    const int bufferSize = ceil(sampleRate * channels * delayTimeSec);

    if (log)
        cout
            << "Sample Rate: " << sampleRate << endl
            << "Frames: " << inFile.frames() << endl
            << "Channels: " << channels << endl
            << "Duration: " << duration << endl
            << "Input: " << inFilePath << endl
            << "Output: " << outFilePath << endl
            << "Delay Time: " << delayTimeSec << endl
            << "Feedback: " << feedbackRatio << endl
            << "Dry: " << dryRatio << endl
            << "Wet: " << wetRatio << endl
            << "Buffer Size: " << bufferSize << endl
            << endl;

    float dataBuffer[bufferSize];
    float prevBuffer[bufferSize];
    memset(prevBuffer, 0, bufferSize * sizeof(float));

    int readCount;
    while ((readCount = (int)inFile.read(dataBuffer, bufferSize)))
    {
        float outDataBuffer[readCount];
        for (int channel = 0; channel < channels; channel++)
        {
            for (int i = channel; i < readCount + (channels - 1); i += channels)
            {
                outDataBuffer[i] = dataBuffer[i] * dryRatio;

                outDataBuffer[i] += ((dataBuffer[i] + prevBuffer[i]) * wetRatio * feedbackRatio);
            }
        }

        outFile.write(outDataBuffer, readCount);

        for (int i = 0; i < readCount; i++)
        {
            prevBuffer[i] = outDataBuffer[i];
        }
    }
}
