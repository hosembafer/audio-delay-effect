#include <filesystem>
#include "delay.hh"

int main()
{
    int delayTime = 250;
    int feedback = 50;
    int dryWet = 50;

    std::filesystem::create_directory("results");
    

    delay("./samples/punch-double-stereo.wav", "./results/punch-result-delay-stereo.wav", delayTime, feedback, dryWet);
    delay("./samples/punch-double-mono.wav", "./results/punch-result-delay-mono.wav", delayTime, feedback, dryWet);

    delay("./samples/2-speakers-1.53-minutes.wav", "./results/speak-result-delay-stereo.wav", delayTime, feedback, dryWet);

    return 0;
}
