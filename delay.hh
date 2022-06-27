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

bool inRange(const int low, const int high, const int x)
{
    return ((x - low) <= (high - low));
}

void delay(
    const std::string &inFilePath,
    const std::string &outFilePath,
    const int delayTime = 500,
    const int feedback = 50,
    const int dryWet = 50,
    const bool log = false)
{
    if (!inRange(0, 1000, delayTime))
        throw std::invalid_argument("Delay Time must be between 0 and 1000");
    if (!inRange(0, 100, feedback))
        throw std::invalid_argument("Feedback must be between 0 and 100");
    if (!inRange(0, 100, dryWet))
        throw std::invalid_argument("Dry/Wet must be between 0 and 100");

    const float delayTimeSec = (float)delayTime / 1000;
    const float feedbackRatio = (float)feedback / 100;
    const float dryRatio = 1.0f - ((float)dryWet / 100);
    const float wetRatio = (float)dryWet / 100;

    SndfileHandle inFile(inFilePath);
    if (inFile.error())
        throw std::runtime_error(std::string("delay: ") + inFile.strError() + " input: " + inFilePath);

    const float sampleRate = (float)inFile.samplerate();
    const float frames = (float)inFile.frames();
    const int channels = inFile.channels();
    const float duration = frames / sampleRate;

    SndfileHandle outFile(outFilePath, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, channels, sampleRate);

    const int bufferSize = ceil(sampleRate * channels * delayTimeSec);

    if (log)
        std::cout
            << "Sample Rate: " << sampleRate << std::endl
            << "Frames: " << inFile.frames() << std::endl
            << "Channels: " << channels << std::endl
            << "Duration: " << duration << std::endl
            << "Input: " << inFilePath << std::endl
            << "Output: " << outFilePath << std::endl
            << "Delay Time: " << delayTimeSec << std::endl
            << "Feedback: " << feedbackRatio << std::endl
            << "Dry: " << dryRatio << std::endl
            << "Wet: " << wetRatio << std::endl
            << "Buffer Size: " << bufferSize << std::endl
            << std::endl;

    float dataBuffer[bufferSize];
    float prevBuffer[bufferSize];
    memset(prevBuffer, 0, bufferSize * sizeof(float));

    sf_count_t readCount;
    while ((readCount = (int)inFile.read(dataBuffer, bufferSize)))
    {
        float outDataBuffer[readCount];
        for (int channel = 0; channel < channels; channel++)
        {
            for (int i = channel; i < readCount + (channels - 1); i += channels)
            {
                // Dry
                outDataBuffer[i] = dataBuffer[i] * dryRatio;

                // Wet
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
