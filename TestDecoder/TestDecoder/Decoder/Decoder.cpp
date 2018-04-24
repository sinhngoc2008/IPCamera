// Decoder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <tchar.h>
#include <evr.h>
#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wmcodecdsp.h>
#include <fstream>
#include <sstream>
#include <string> 
#include <iostream>

#include "MFUltility.h"

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")

#define MAX_BUFFER  65536*8

using namespace std; 

int _tmain(int argc, _TCHAR* argv[])
{
	const int SAMPLE_COUNT = 30;
	//const int VIDEO_SAMPLE_WIDTH = 640;	// Note the frmae width and height need to be set based on the frame size in the MP4 file.
	//const int VIDEO_SAMPLE_HEIGHT = 360;

	const int VIDEO_SAMPLE_WIDTH = 1920;	// Note the frmae width and height need to be set based on the frame size in the MP4 file.
	const int VIDEO_SAMPLE_HEIGHT = 1080;

	//wchar_t *filename = L"D:\\Project\\IPCamera\\IPCamera\\TestDecoder\\TestDecoder\\Debug\\big_buck_bunny.mp4";
	//wchar_t *filename = L"D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/Debug/big_buck_bunny.mp4";
	wchar_t *filename = L"D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/inputvideo/video1.mp4";
	//wchar_t *filename = L"D:/Project/IPCamera/TMP/mediafoundationsamples/MediaFiles/big_buck_bunny.mp4";

	IMFSourceReader *m_pReader;

	IMFSourceResolver *pSourceResolver = NULL;
	IUnknown* uSource = NULL;
	IMFMediaSource *mediaFileSource = NULL;
	IMFAttributes *pVideoReaderAttributes = NULL;
	IMFSourceReader *pSourceReader = NULL;
	MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
	IMFMediaType *pFileVideoMediaType = NULL;

	IUnknown *spDecTransformUnk = NULL;
	IMFTransform *pDecoderTransform = NULL; // This is H264 Decoder MFT.
	IMFMediaType *pDecInputMediaType = NULL, *pDecOutputMediaType = NULL;
	DWORD mftStatus = 0;


	IMFByteStream *stream = NULL;
	IMFSourceResolver *sourceresolve = NULL;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	MFStartup(MF_VERSION);
	

	// Set up the reader for the file.
	CHECK_HR(MFCreateSourceResolver(&pSourceResolver), "MFCreateSourceResolver failed.\n");


	CHECK_HR(pSourceResolver->CreateObjectFromURL(
		filename,					// URL of the source.
		MF_RESOLUTION_MEDIASOURCE,  // Create a source object.MF_RESOLUTION_MEDIASOURCE
		NULL,                       // Optional property store.
		&ObjectType,				// Receives the created object type. 
		&uSource					// Receives a pointer to the media source.
		), "Failed to create media source resolver for file.\n");
	 
	CHECK_HR(uSource->QueryInterface(IID_PPV_ARGS(&mediaFileSource)), "Failed to create media file source.\n");

	CHECK_HR(MFCreateAttributes(&pVideoReaderAttributes, 2), "Failed to create attributes object for video reader.\n");

	CHECK_HR(pVideoReaderAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID),
		"Failed to set dev source attribute type for reader config.\n");

	CHECK_HR(pVideoReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1), "Failed to set enable video processing attribute type for reader config.\n");

	CHECK_HR(MFCreateSourceReaderFromMediaSource(mediaFileSource, pVideoReaderAttributes, &pSourceReader),
		"Error creating media source reader.\n");

	//CHECK_HR(pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pFileVideoMediaType), "Error retrieving current media type from first video stream.\n");

#if 0


	// Create H.264 decoder.
	CHECK_HR(CoCreateInstance(CLSID_CMSH264DecoderMFT, NULL, CLSCTX_INPROC_SERVER,
		IID_IUnknown, (void**)&spDecTransformUnk), "Failed to create H264 decoder MFT.\n");

	//Init MF Decoder Transform
	CHECK_HR(spDecTransformUnk->QueryInterface(IID_PPV_ARGS(&pDecoderTransform)), "Failed to get IMFTransform interface from H264 decoder MFT object.\n");
	//Set MediaType Input
	MFCreateMediaType(&pDecInputMediaType);

	//Connect source media to InputMediaType
	// setup mediatype from source file
	CHECK_HR(pFileVideoMediaType->CopyAllItems(pDecInputMediaType), "Error copying media type attributes to decoder input media type.\n");
#else	
	// Create H.264 decoder.
	CHECK_HR(CoCreateInstance(CLSID_CMSH264DecoderMFT, NULL, CLSCTX_INPROC_SERVER,
		IID_IUnknown, (void**)&spDecTransformUnk), "Failed to create H264 decoder MFT.\n");

	//Init MF Decoder Transform
	CHECK_HR(spDecTransformUnk->QueryInterface(IID_PPV_ARGS(&pDecoderTransform)), "Failed to get IMFTransform interface from H264 decoder MFT object.\n");

	//Set MediaType Input
	MFCreateMediaType(&pDecInputMediaType);

	// setup media type manualy 
	pDecInputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	pDecInputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);

	pDecInputMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	MFSetAttributeSize(pDecInputMediaType, MF_MT_FRAME_SIZE, VIDEO_SAMPLE_WIDTH, VIDEO_SAMPLE_HEIGHT);
	unsigned __int32 fixedSampleSize = VIDEO_SAMPLE_WIDTH*(16 * ((VIDEO_SAMPLE_HEIGHT + 15) / 16)) + VIDEO_SAMPLE_WIDTH*(VIDEO_SAMPLE_HEIGHT / 2);//for Y, U and V
	pDecInputMediaType->SetUINT32(MF_MT_SAMPLE_SIZE, fixedSampleSize);
	pDecInputMediaType->SetUINT32(MF_MT_DEFAULT_STRIDE, VIDEO_SAMPLE_WIDTH);
	pDecInputMediaType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
	pDecInputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
	MFSetAttributeRatio(pDecInputMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
#endif

	//Setup input media for transform
	CHECK_HR(pDecoderTransform->SetInputType(0, pDecInputMediaType, 0), "Failed to set input media type on H.264 decoder MFT.\n");

	//Set MediaType Output
	MFCreateMediaType(&pDecOutputMediaType);
	pDecOutputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	pDecOutputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_IYUV);
	CHECK_HR(MFSetAttributeSize(pDecOutputMediaType, MF_MT_FRAME_SIZE, VIDEO_SAMPLE_WIDTH, VIDEO_SAMPLE_HEIGHT), "Failed to set frame size on H264 MFT out type.\n");
	CHECK_HR(MFSetAttributeRatio(pDecOutputMediaType, MF_MT_FRAME_RATE, 30, 1), "Failed to set frame rate on H264 MFT out type.\n");
	CHECK_HR(MFSetAttributeRatio(pDecOutputMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), "Failed to set aspect ratio on H264 MFT out type.\n");
	pDecOutputMediaType->SetUINT32(MF_MT_INTERLACE_MODE, 2);

	CHECK_HR(pDecoderTransform->SetOutputType(0, pDecOutputMediaType, 0), "Failed to set output media type on H.264 decoder MFT.\n");

	CHECK_HR(pDecoderTransform->GetInputStatus(0, &mftStatus), "Failed to get input status from H.264 decoder MFT.\n");
	if (MFT_INPUT_STATUS_ACCEPT_DATA != mftStatus) {
		printf("H.264 decoder MFT is not accepting data.\n");
		goto done;
	}

	CHECK_HR(pDecoderTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL), "Failed to process FLUSH command on H.264 decoder MFT.\n");
	CHECK_HR(pDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL), "Failed to process BEGIN_STREAMING command on H.264 decoder MFT.\n");
	CHECK_HR(pDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL), "Failed to process START_OF_STREAM command on H.264 decoder MFT.\n");

	// Process Input.
	MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
	DWORD processOutputStatus = 0;
	IMFSample *videoSample = NULL, *reConstructedVideoSample = NULL, *srcVideoSample = NULL;
	DWORD streamIndex, flags, bufferCount;
	LONGLONG llVideoTimeStamp, llSampleDuration, yuvVideoTimeStamp, yuvSampleDuration;
	HRESULT mftProcessInput = S_OK;
	HRESULT mftProcessOutput = S_OK;
	MFT_OUTPUT_STREAM_INFO StreamInfo;
	IMFSample *mftOutSample = NULL;
	IMFMediaBuffer *pBuffer = NULL, *reConstructedBuffer = NULL, *srcBuffer = NULL;
	int sampleCount = 0;
	DWORD mftOutFlags;
	//DWORD srcBufLength;
	DWORD sampleFlags;
	LONGLONG reconVideoTimeStamp, reconSampleDuration;
	DWORD reconSampleFlags;

	// Read file *.mp4 and Save one frame to file 
#if 0
	while (sampleCount <= 160)
	{
		if (sampleCount >= 0)
		{ 
			CHECK_HR(pSourceReader->ReadSample(
				MF_SOURCE_READER_FIRST_VIDEO_STREAM,
				0,                              // Flags.
				&streamIndex,                   // Receives the actual stream index. 
				&flags,                         // Receives status flags.
				&llVideoTimeStamp,              // Receives the time stamp.
				&videoSample                    // Receives the sample or NULL.
				), "Error reading video sample.");

			if (videoSample)
			{
				printf("Processing sample %i.\n", sampleCount);

				CHECK_HR(videoSample->GetSampleDuration(&llSampleDuration), "Error getting video sample duration.\n");
				videoSample->GetSampleFlags(&flags);
				videoSample->GetTotalLength(&bufferCount);

				printf("Sample time %I64d, sample duration %I64d, sample flags %d, buffer count %i. COUNT: %d\n", llVideoTimeStamp, llSampleDuration, flags, bufferCount, sampleCount);

				IMFMediaBuffer *srcBuf = NULL;
				byte *byteBuffer;
				DWORD buffCurrLen = 0;
				DWORD buffMaxLen = 0;

				CHECK_HR(videoSample->ConvertToContiguousBuffer(&srcBuf), "ConvertToContiguousBuffer failed.\n");

				srcBuf->Lock(&byteBuffer, &buffMaxLen, &buffCurrLen);

				std::string count;
				std::string name;
				std::string frameurl = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/h264frames/";
				count = std::to_string(sampleCount);

				name = "h264frames" + count + ".endc";

				printf("srcBufLength: %d \n", bufferCount);
				printf("byteBuffer: %s \n", byteBuffer);

				std::ofstream outputBuffer1(frameurl + name, std::ios::out | std::ios::binary);
				outputBuffer1.write((char *)byteBuffer, bufferCount);
				outputBuffer1.flush();
				outputBuffer1.close();

				CHECK_HR(srcBuf->Unlock(), "Error unlocking source buffer.\n");

			}
			sampleCount++;
		}
	}
#endif
	// Read one frame from file and setup the input for transform
#if 1 

		int count = 0;
		HRESULT frameProcessOutput = S_OK;
		//const char *inputurl = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/h264frames/";
		const char *inputurl = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/captureframe/";

		char *imgvurl = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/yuvimg/";
		char *output;
		 
		memset(&outputDataBuffer, 0, sizeof outputDataBuffer);

		while (sampleCount <= 160)
		{ 
			// load file to buffer 	
			//char *h264framefile = "h264frames0.endc";
			char *yuvname = "rawh264img.yuv";

			output = (char *)malloc(strlen(imgvurl) + strlen(imgvurl) + 1);
			strcpy(output, imgvurl);
			strcat(output, yuvname);


			FILE *_h264file = NULL;
			uint8_t buffer[MAX_BUFFER];
			long buffer_size = 0;

			//std::string name = (string)inputurl + "h264frames" + to_string(sampleCount) + ".endc";
			//std::string name = (string)inputurl + "rawframes" + to_string(sampleCount) + ".endc";
			std::string name = (string)inputurl + to_string(sampleCount)+ "_rtsp_capture.endc";

			printf("INPUT FILE NAME: %s\n", name.c_str());

			_h264file = fopen(name.c_str(), "rb");

			if (!_h264file) return 0;

			// obtain file size:
			long lSize;
			fseek(_h264file, 0, SEEK_END);
			lSize = ftell(_h264file);
			rewind(_h264file);

			printf("Size of file: %ld\n", lSize);

			buffer_size = fread(buffer, 1, lSize, _h264file);
			//fflush(_h264file);

			fclose(_h264file); 

			if (sampleCount >= 0)
			{     
				printf("Processing sample %i.\n", sampleCount);
					
				// Extrtact and then re-construct the sample to simulate processing encoded H264 frames received outside of MF.
				IMFMediaBuffer *srcBuf = NULL;
				DWORD srcBufLength;
				byte *srcByteBuffer;
				DWORD srcBuffCurrLen = 0;
				DWORD srcBuffMaxLen = 0;
				 
				//// Now re-constuct.
				MFCreateSample(&srcVideoSample);
				CHECK_HR(MFCreateMemoryBuffer(lSize, &srcBuffer), "Failed to create memory buffer.\n");
				CHECK_HR(srcVideoSample->AddBuffer(srcBuffer), "Failed to add buffer to re-constructed sample.\n");

				byte *reconByteBuffer;
				DWORD reconBuffCurrLen = 0;
				DWORD reconBuffMaxLen = 0;
				CHECK_HR(srcBuffer->Lock(&reconByteBuffer, &reconBuffMaxLen, &reconBuffCurrLen), "Error locking recon buffer.\n");

				memcpy(reconByteBuffer, buffer, lSize);

				CHECK_HR(srcBuffer->Unlock(), "Error unlocking recon buffer.\n");
				srcBuffer->SetCurrentLength(lSize);

				CHECK_HR(pDecoderTransform->ProcessInput(0, srcVideoSample, 0), "The H264 decoder ProcessInput call failed.\n");

				CHECK_HR(pDecoderTransform->GetOutputStreamInfo(0, &StreamInfo), "Failed to get output stream info from H264 MFT.\n");

				printf("STREAM INFO SIZE: %ld \n", StreamInfo.cbSize);
				printf("lSize SIZE: %ld \n", lSize);

				while (true)
				{
					CHECK_HR(MFCreateSample(&mftOutSample), "Failed to create MF sample.\n");
					//CHECK_HR(MFCreateMemoryBuffer(lSize, &pBuffer), "Failed to create memory buffer.\n");
					CHECK_HR(MFCreateMemoryBuffer(StreamInfo.cbSize, &pBuffer), "Failed to create memory buffer.\n");
					CHECK_HR(mftOutSample->AddBuffer(pBuffer), "Failed to add sample to buffer.\n");
					outputDataBuffer.dwStreamID = 0;
					outputDataBuffer.dwStatus = 0;
					outputDataBuffer.pEvents = NULL;
					outputDataBuffer.pSample = mftOutSample;

					mftProcessOutput = pDecoderTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);

					if (mftProcessOutput != MF_E_TRANSFORM_NEED_MORE_INPUT)
					{ 
						IMFMediaBuffer *buf = NULL;
						DWORD bufLength;
						CHECK_HR(mftOutSample->ConvertToContiguousBuffer(&buf), "ConvertToContiguousBuffer failed.\n");
						CHECK_HR(buf->GetCurrentLength(&bufLength), "Get buffer length failed.\n"); 

						byte *byteBuffer;
						DWORD buffCurrLen = 0;
						DWORD buffMaxLen = 0;
						buf->Lock(&byteBuffer, &buffMaxLen, &buffCurrLen);
						/*outputBuffer.write((char *)byteBuffer, bufLength);
						outputBuffer.flush();*/

						printf("Stream SIZE: %ld bufLength: %ld, buffCurrLen: %ld \n", StreamInfo.cbSize, bufLength, buffCurrLen);

						std::string count;
						std::string name;
						//std::string yuvurl = "D:/Project/IPCamera/TMP/mediafoundationsamples/MFMP4ToYUVWithMFT/output/yuvfile/";
						std::string yuvurl = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/yuvimg/";
					
						//std::string imgvurl = "D:/Project/IPCamera/TMP/mediafoundationsamples/MFMP4ToYUVWithMFT/output/image/";
						std::string imgvurl = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/imgjpeg/";
						count = std::to_string(sampleCount);

						name = "rawframes" + count + ".yuv";

						if ((sampleCount % 1) == 0)
						{
							std::cout << name;
							printf("Write to file >>>>>>>>>>>>>>>>>>>>>>> %s \n", name.c_str());


							std::ofstream outputBuffer1(yuvurl + name, std::ios::out | std::ios::binary);
							outputBuffer1.write((char *)byteBuffer, bufLength);
							outputBuffer1.flush();
							outputBuffer1.close();

							std::string instruction = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/ffmpeg.exe -vcodec rawvideo -s 1920x1080 -pix_fmt yuv420p -i " + yuvurl + name + " -vframes 1 " + imgvurl + name + ".jpeg -y";

							const char * ins = instruction.c_str();
							printf("INSTRUCTION: %s\n", ins);

							system(ins);

							std::string delinst = "del " + yuvurl + name;
							const char * delins = delinst.c_str();
							//system(delins); 
						} 
					}
					else 
					{
						break;
					}

					mftOutSample->Release();
				}  
			} 
			sampleCount++; 
		}

#endif

done: 
	printf("finished.\n");
	getchar();

	return 0;
} 