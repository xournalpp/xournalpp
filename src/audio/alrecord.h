#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "common/alhelpers.h"

#include <errno.h>
//#include "../common/threads.h"

#if defined(_WIN64)
#define SZFMT "%I64u"
#elif defined(_WIN32)
#define SZFMT "%u"
#else
#define SZFMT "%zu"
#endif


#if defined(_MSC_VER) && (_MSC_VER < 1900)
static float msvc_strtof(const char *str, char **end)
{ return (float)strtod(str, end); }
#define strtof msvc_strtof
#endif

typedef struct Recorder {
    ALCdevice *mDevice;

    FILE *mFile;
    long mDataSizeOffset;
    ALuint mDataSize;
    float mRecTime;

    int mChannels;
    int mBits;
    int mSampleRate;
    ALuint mFrameSize;
    ALbyte *mBuffer;
    ALsizei mBufferSize;
} Recorder;

int record();
int rec2(int argc, char **argv);