/*
 * OpenAL Recording Example
 *
 * Copyright (c) 2017 by Chris Robinson <chris.kcat@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* This file contains a relatively simple recorder. */

#include "alrecord.h"


static void fwrite16le(ALushort val, FILE *f)
{
    ALubyte data[2] = { val&0xff, (val>>8)&0xff };
    fwrite(data, 1, 2, f);
}

static void fwrite32le(ALuint val, FILE *f)
{
    ALubyte data[4] = { val&0xff, (val>>8)&0xff, (val>>16)&0xff, (val>>24)&0xff };
    fwrite(data, 1, 4, f);
}

int record()
{
    rec2(0,NULL);
}

int rec2(int argc, char **argv)
{
    static const char optlist[] =
"    --channels/-c <channels>  Set channel count (1 or 2)\n"
"    --bits/-b <bits>          Set channel count (8, 16, or 32)\n"
"    --rate/-r <rate>          Set sample rate (8000 to 96000)\n"
"    --time/-t <time>          Time in seconds to record (1 to 10)\n"
"    --outfile/-o <filename>   Output filename (default: record.wav)";
    const char *fname = "record.wav";
    const char *devname = NULL;
    const char *progname;
    Recorder recorder;
    long total_size;
    ALenum format;
    ALCenum err;
/*
    progname = argv[0];
    if(argc < 2)
    {
        fprintf(stderr, "Record from a device to a wav file.\n\n"
                "Usage: %s [-device <name>] [options...]\n\n"
                "Available options:\n%s\n", progname, optlist);
        return 0;
    }
*/
    recorder.mDevice = NULL;
    recorder.mFile = NULL;
    recorder.mDataSizeOffset = 0;
    recorder.mDataSize = 0;
    recorder.mRecTime = 4.0f;
    recorder.mChannels = 1;
    recorder.mBits = 16;
    recorder.mSampleRate = 44100;
    recorder.mFrameSize = recorder.mChannels * recorder.mBits / 8;
    recorder.mBuffer = NULL;
    recorder.mBufferSize = 0;
/*
    argv++; argc--;
    if(argc > 1 && strcmp(argv[0], "-device") == 0)
    {
        devname = argv[1];
        argv += 2;
        argc -= 2;
    }

    while(argc > 0)
    {
        char *end;
        if(strcmp(argv[0], "--") == 0)
            break;
        else if(strcmp(argv[0], "--channels") == 0 || strcmp(argv[0], "-c") == 0)
        {
            if(argc < 2)
            {
                fprintf(stderr, "Missing argument for option: %s\n", argv[0]);
                return 1;
            }

            recorder.mChannels = strtol(argv[1], &end, 0);
            if((recorder.mChannels != 1 && recorder.mChannels != 2) || (end && *end != '\0'))
            {
                fprintf(stderr, "Invalid channels: %s\n", argv[1]);
                return 1;
            }
            argv += 2;
            argc -= 2;
        }
        else if(strcmp(argv[0], "--bits") == 0 || strcmp(argv[0], "-b") == 0)
        {
            if(argc < 2)
            {
                fprintf(stderr, "Missing argument for option: %s\n", argv[0]);
                return 1;
            }

            recorder.mBits = strtol(argv[1], &end, 0);
            if((recorder.mBits != 8 && recorder.mBits != 16 && recorder.mBits != 32) ||
               (end && *end != '\0'))
            {
                fprintf(stderr, "Invalid bit count: %s\n", argv[1]);
                return 1;
            }
            argv += 2;
            argc -= 2;
        }
        else if(strcmp(argv[0], "--rate") == 0 || strcmp(argv[0], "-r") == 0)
        {
            if(argc < 2)
            {
                fprintf(stderr, "Missing argument for option: %s\n", argv[0]);
                return 1;
            }

            recorder.mSampleRate = strtol(argv[1], &end, 0);
            if(!(recorder.mSampleRate >= 8000 && recorder.mSampleRate <= 96000) || (end && *end != '\0'))
            {
                fprintf(stderr, "Invalid sample rate: %s\n", argv[1]);
                return 1;
            }
            argv += 2;
            argc -= 2;
        }
        else if(strcmp(argv[0], "--time") == 0 || strcmp(argv[0], "-t") == 0)
        {
            if(argc < 2)
            {
                fprintf(stderr, "Missing argument for option: %s\n", argv[0]);
                return 1;
            }

            recorder.mRecTime = strtof(argv[1], &end);
            if(!(recorder.mRecTime >= 1.0f && recorder.mRecTime <= 10.0f) || (end && *end != '\0'))
            {
                fprintf(stderr, "Invalid record time: %s\n", argv[1]);
                return 1;
            }
            argv += 2;
            argc -= 2;
        }
        else if(strcmp(argv[0], "--outfile") == 0 || strcmp(argv[0], "-o") == 0)
        {
            if(argc < 2)
            {
                fprintf(stderr, "Missing argument for option: %s\n", argv[0]);
                return 1;
            }

            fname = argv[1];
            argv += 2;
            argc -= 2;
        }
        else if(strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0)
        {
            fprintf(stderr, "Record from a device to a wav file.\n\n"
                    "Usage: %s [-device <name>] [options...]\n\n"
                    "Available options:\n%s\n", progname, optlist);
            return 0;
        }
        else
        {
            fprintf(stderr, "Invalid option '%s'.\n\n"
                    "Usage: %s [-device <name>] [options...]\n\n"
                    "Available options:\n%s\n", argv[0], progname, optlist);
            return 0;
        }
    }
*/
    recorder.mFrameSize = recorder.mChannels * recorder.mBits / 8;

    format = AL_NONE;
    if(recorder.mChannels == 1)
    {
        if(recorder.mBits == 8)
            format = AL_FORMAT_MONO8;
        else if(recorder.mBits == 16)
            format = AL_FORMAT_MONO16;
        else if(recorder.mBits == 32)
            format = AL_FORMAT_MONO_FLOAT32;
    }
    else if(recorder.mChannels == 2)
    {
        if(recorder.mBits == 8)
            format = AL_FORMAT_STEREO8;
        else if(recorder.mBits == 16)
            format = AL_FORMAT_STEREO16;
        else if(recorder.mBits == 32)
            format = AL_FORMAT_STEREO_FLOAT32;
    }

    recorder.mDevice = alcCaptureOpenDevice(devname, recorder.mSampleRate, format, 32768);
    if(!recorder.mDevice)
    {
        fprintf(stderr, "Failed to open %s, %s %d-bit, %s, %dhz (%d samples)\n",
            devname ? devname : "default device",
            (recorder.mBits == 32) ? "Float" :
            (recorder.mBits !=  8) ? "Signed" : "Unsigned", recorder.mBits,
            (recorder.mChannels == 1) ? "Mono" : "Stereo", recorder.mSampleRate,
            32768
        );
        return 1;
    }
    fprintf(stderr, "Opened \"%s\"\n", alcGetString(
        recorder.mDevice, ALC_CAPTURE_DEVICE_SPECIFIER
    ));

    recorder.mFile = fopen(fname, "wb");
    if(!recorder.mFile)
    {
        fprintf(stderr, "Failed to open '%s' for writing\n", fname);
        alcCaptureCloseDevice(recorder.mDevice);
        return 1;
    }

    fputs("RIFF", recorder.mFile);
    fwrite32le(0xFFFFFFFF, recorder.mFile); // 'RIFF' header len; filled in at close

    fputs("WAVE", recorder.mFile);

    fputs("fmt ", recorder.mFile);
    fwrite32le(18, recorder.mFile); // 'fmt ' header len

    // 16-bit val, format type id (1 = integer PCM, 3 = float PCM)
    fwrite16le((recorder.mBits == 32) ? 0x0003 : 0x0001, recorder.mFile);
    // 16-bit val, channel count
    fwrite16le(recorder.mChannels, recorder.mFile);
    // 32-bit val, frequency
    fwrite32le(recorder.mSampleRate, recorder.mFile);
    // 32-bit val, bytes per second
    fwrite32le(recorder.mSampleRate * recorder.mFrameSize, recorder.mFile);
    // 16-bit val, frame size
    fwrite16le(recorder.mFrameSize, recorder.mFile);
    // 16-bit val, bits per sample
    fwrite16le(recorder.mBits, recorder.mFile);
    // 16-bit val, extra byte count
    fwrite16le(0, recorder.mFile);

    fputs("data", recorder.mFile);
    fwrite32le(0xFFFFFFFF, recorder.mFile); // 'data' header len; filled in at close

    recorder.mDataSizeOffset = ftell(recorder.mFile) - 4;
    if(ferror(recorder.mFile) || recorder.mDataSizeOffset < 0)
    {
        fprintf(stderr, "Error writing header: %s\n", strerror(errno));
        fclose(recorder.mFile);
        alcCaptureCloseDevice(recorder.mDevice);
        return 1;
    }

    fprintf(stderr, "Recording '%s', %s %d-bit, %s, %dhz (%g second%s)\n", fname,
        (recorder.mBits == 32) ? "Float" :
        (recorder.mBits !=  8) ? "Signed" : "Unsigned", recorder.mBits,
        (recorder.mChannels == 1) ? "Mono" : "Stereo", recorder.mSampleRate,
        recorder.mRecTime, (recorder.mRecTime != 1.0f) ? "s" : ""
    );

    alcCaptureStart(recorder.mDevice);
    while((double)recorder.mDataSize/(double)recorder.mSampleRate < recorder.mRecTime &&
          (err=alcGetError(recorder.mDevice)) == ALC_NO_ERROR && !ferror(recorder.mFile))
    {
        ALCint count = 0;
        fprintf(stderr, "\rCaptured %u samples", recorder.mDataSize);
        alcGetIntegerv(recorder.mDevice, ALC_CAPTURE_SAMPLES, 1, &count);
        if(count < 1)
        {
	    nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
            //al_nssleep(10000000);
            continue;
        }
        if(count > recorder.mBufferSize)
        {
            ALbyte *data = calloc(recorder.mFrameSize, count);
            free(recorder.mBuffer);
            recorder.mBuffer = data;
            recorder.mBufferSize = count;
        }
        alcCaptureSamples(recorder.mDevice, recorder.mBuffer, count);
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
        /* Byteswap multibyte samples on big-endian systems (wav needs little-
         * endian, and OpenAL gives the system's native-endian).
         */
        if(recorder.mBits == 16)
        {
            ALCint i;
            for(i = 0;i < count*recorder.mChannels;i++)
            {
                ALbyte b = recorder.mBuffer[i*2 + 0];
                recorder.mBuffer[i*2 + 0] = recorder.mBuffer[i*2 + 1];
                recorder.mBuffer[i*2 + 1] = b;
            }
        }
        else if(recorder.mBits == 32)
        {
            ALCint i;
            for(i = 0;i < count*recorder.mChannels;i++)
            {
                ALbyte b0 = recorder.mBuffer[i*4 + 0];
                ALbyte b1 = recorder.mBuffer[i*4 + 1];
                recorder.mBuffer[i*4 + 0] = recorder.mBuffer[i*4 + 3];
                recorder.mBuffer[i*4 + 1] = recorder.mBuffer[i*4 + 2];
                recorder.mBuffer[i*4 + 2] = b1;
                recorder.mBuffer[i*4 + 3] = b0;
            }
        }
#endif
        recorder.mDataSize += (ALuint)fwrite(recorder.mBuffer, recorder.mFrameSize, count,
                                             recorder.mFile);
    }
    alcCaptureStop(recorder.mDevice);
    fprintf(stderr, "\rCaptured %u samples\n", recorder.mDataSize);
    if(err != ALC_NO_ERROR)
        fprintf(stderr, "Got device error 0x%04x: %s\n", err, alcGetString(recorder.mDevice, err));

    alcCaptureCloseDevice(recorder.mDevice);
    recorder.mDevice = NULL;

    free(recorder.mBuffer);
    recorder.mBuffer = NULL;
    recorder.mBufferSize = 0;

    total_size = ftell(recorder.mFile);
    if(fseek(recorder.mFile, recorder.mDataSizeOffset, SEEK_SET) == 0)
    {
        fwrite32le(recorder.mDataSize*recorder.mFrameSize, recorder.mFile);
        if(fseek(recorder.mFile, 4, SEEK_SET) == 0)
            fwrite32le(total_size - 8, recorder.mFile);
    }

    fclose(recorder.mFile);
    recorder.mFile = NULL;

    return 0;
}
