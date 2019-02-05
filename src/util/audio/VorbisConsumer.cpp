#include "VorbisConsumer.h"

VorbisConsumer::VorbisConsumer(Settings* settings, AudioQueue<float>* audioQueue)
		: settings(settings),
		  audioQueue(audioQueue)
{
	XOJ_INIT_TYPE(VorbisConsumer);
}

VorbisConsumer::~VorbisConsumer()
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	XOJ_RELEASE_TYPE(VorbisConsumer);
}

bool VorbisConsumer::start(string filename, unsigned int inputChannels)
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	this->consumerThread = new std::thread(
			[&, filename, inputChannels]
			{
				std::unique_lock<std::mutex> lock(audioQueue->syncMutex());

				std::ofstream outputFile(filename, std::ios::out | std::ios::binary);
				try
				{
					outputFile.exceptions(outputFile.failbit);
				}
				catch (const std::ios_base::failure& e)
				{
					g_warning("VorbisConsumer: output file \"%s\" could not be opened", filename.c_str());
					return;
				}

				ogg_stream_state os;
				ogg_page og;
				ogg_packet op;
				vorbis_info vi;
				vorbis_comment vc;
				vorbis_dsp_state vd;
				vorbis_block vb;

				vorbis_info_init(&vi);
				int vorbis_error = 0;
				vorbis_error = vorbis_encode_init_vbr(&vi, (long) inputChannels, (long) this->settings->getAudioSampleRate(), 0.1);
				if (vorbis_error)
				{
					g_warning("VorbisConsumer: Failed to initialize encoder");
				}

				vorbis_comment_init(&vc);
				vorbis_comment_add_tag(&vc, "ENCODER", "Xournal++");

				vorbis_analysis_init(&vd, &vi);
				vorbis_block_init(&vd, &vb);

				ogg_stream_init(&os, 0);

				ogg_packet header;
				ogg_packet header_comm;
				ogg_packet header_code;

				vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
				ogg_stream_packetin(&os, &header);
				ogg_stream_packetin(&os, &header_comm);
				ogg_stream_packetin(&os, &header_code);

				while (true)
				{
					int result = ogg_stream_flush(&os, &og);
					if (result == 0)
					{
						break;
					}
					outputFile.write(reinterpret_cast<const char*>(og.header), og.header_len);
					outputFile.write(reinterpret_cast<const char*>(og.body), og.body_len);
				}

				float buffer[64 * inputChannels];
				int bufferLength;

				while (!(this->stopConsumer || (audioQueue->hasStreamEnded() && audioQueue->empty())))
				{
					audioQueue->waitForNewElements(lock);

					while (!audioQueue->empty())
					{
						audioQueue->pop(buffer, &bufferLength, 64ul * inputChannels, inputChannels);

						if (bufferLength > 0)
						{
							float** vorbisBuffer = vorbis_analysis_buffer(&vd, bufferLength / inputChannels);

							for (int i = 0; i < bufferLength / inputChannels; i++)
							{
								for (int j = 0; j < inputChannels; j++)
								{
									vorbisBuffer[j][i] = buffer[i * inputChannels + j];
								}
							}

							vorbis_analysis_wrote(&vd, bufferLength / inputChannels);
						} else
						{
							vorbis_analysis_wrote(&vd, 0);
						}

						while (vorbis_analysis_blockout(&vd, &vb) == 1)
						{
							vorbis_analysis(&vb, nullptr);
							vorbis_bitrate_addblock(&vb);

							while (vorbis_bitrate_flushpacket(&vd, &op))
							{
								ogg_stream_packetin(&os, &op);

								while (true)
								{
									int result = ogg_stream_pageout(&os, &og);
									if (result == 0 || ogg_page_eos(&og))
									{
										break;
									}
									outputFile.write(reinterpret_cast<const char*>(og.header), og.header_len);
									outputFile.write(reinterpret_cast<const char*>(og.body), og.body_len);
								}
							}
						}
					}
				}

				ogg_stream_clear(&os);
				vorbis_block_clear(&vb);
				vorbis_dsp_clear(&vd);
				vorbis_comment_clear(&vc);
				vorbis_info_clear(&vi);

				outputFile.close();
			});
	return true;
}

void VorbisConsumer::join()
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	// Join the consumer thread to wait for completion
	if (this->consumerThread && this->consumerThread->joinable())
	{
		this->consumerThread->join();
	}
}

void VorbisConsumer::stop()
{
	XOJ_CHECK_TYPE(VorbisConsumer);

	// Stop consumer
	this->audioQueue->signalEndOfStream();

	// Wait for consumer to finish
	join();
}
