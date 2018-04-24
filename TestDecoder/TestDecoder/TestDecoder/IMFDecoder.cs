using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;

using MediaFoundation;
using MediaFoundation.ReadWrite;
using MediaFoundation.EVR;
using MediaFoundation.Misc;
using MediaFoundation.Transform;

namespace TestDecoder
{
    [ComImport, Guid("62CE7E72-4C71-4D20-B15D-452831A87D9D")]
    class ResamplerMediaComObject
    {
    } 
    class IMFDecoder
    {        
        // Define video size
        const int VIDEO_SAMPLE_WIDTH = 1920;
        const int VIDEO_SAMPLE_HEIGHT = 1080;
         

        string inputfile = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/inputvideo/video1.mp4";


        IMFSourceResolver sourceresolver;
        object usource;
        IMFMediaSource sourcemediafile;
        IMFAttributes sourcevideoreaderattribute;
        
        IMFSourceReader  sourcereader;
        IMFMediaType sourcemeidatype;
         
        IMFTransform  decodertransform; // This is H264 Decoder MFT.
	    IMFMediaType decinputmediatype, decoutputmediatype;

        MFTInputStatusFlags mftStatus = 0;

        public IMFDecoder()
        {
            // Set up the reader for the file.
            //sourcemediafile = CreateMediaSource(inputfile);
            //sourcereader = CreateSourceReader(sourcemediafile);
            CreateH264Decoder(); 
        }
        private IMFMediaSource CreateMediaSource(string sURL)
        {

            IMFSourceResolver pSourceResolver;
            object pSource;
            IMFMediaSource mediasource;

            // Create the source resolver.
            HResult hr = MFExtern.MFCreateSourceResolver(out pSourceResolver);
            MFError.ThrowExceptionForHR(hr);

            try
            {
                // Use the source resolver to create the media source.
                MFObjectType ObjectType = MFObjectType.Invalid;

                hr = pSourceResolver.CreateObjectFromURL(
                        sURL,                       // URL of the source.
                        MFResolution.MediaSource,   // Create a source object.
                        null,                       // Optional property store.
                        out ObjectType,             // Receives the created object type.
                        out pSource                 // Receives a pointer to the media source.
                    );
                MFError.ThrowExceptionForHR(hr);

                // Get the IMFMediaSource interface from the media source.
                mediasource = (IMFMediaSource)pSource;
            }
            finally
            {
                // Clean up
                Marshal.ReleaseComObject(pSourceResolver);
            }
            return mediasource;
        }

        private IMFSourceReader CreateSourceReader(IMFMediaSource mfs)
        {
            IMFSourceReader reader = null;
            try
            {
                MFExtern.MFCreateAttributes(out sourcevideoreaderattribute, 2);

                sourcevideoreaderattribute.SetGUID(MFAttributesClsid.MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, CLSID.MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
                sourcevideoreaderattribute.SetUINT32(MFAttributesClsid.MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1);


                MFExtern.MFCreateSourceReaderFromMediaSource(mfs, sourcevideoreaderattribute, out reader);
                //reader.GetCurrentMediaType((int)__MIDL___MIDL_itf_mfreadwrite_0000_0001_0001.MF_SOURCE_READER_FIRST_VIDEO_STREAM, out sourcemeidatype);
            }
            catch(Exception e)
            {
                Debug.WriteLine("ERROR CreateSourceReader", e.StackTrace);
            } 
            return reader;
        }
        
        private void CreateH264Decoder()
        {
            HResult hr;
            // create H.264 decoder

            var comobject = new ResamplerMediaComObject();
            decodertransform = (IMFTransform)comobject;

            // setup input media type for decoder
            MFExtern.MFCreateMediaType(out decinputmediatype);
 
            // setup media type manualy 
            IMFMediaType testdecinputmediatype, testdecoutputmediatype;

	        decinputmediatype.SetGUID(MFAttributesClsid.MF_MT_MAJOR_TYPE, MFMediaType.Video);
	        decinputmediatype.SetGUID(MFAttributesClsid.MF_MT_SUBTYPE, MFMediaType.H264); 
	        decinputmediatype.SetUINT32(MFAttributesClsid.MF_MT_INTERLACE_MODE , (int)MFVideoInterlaceMode.Progressive);

	        MFExtern.MFSetAttributeSize(decinputmediatype, MFAttributesClsid.MF_MT_FRAME_SIZE, VIDEO_SAMPLE_WIDTH, VIDEO_SAMPLE_HEIGHT);
	        uint fixedSampleSize = VIDEO_SAMPLE_WIDTH*(16 * ((VIDEO_SAMPLE_HEIGHT + 15) / 16)) + VIDEO_SAMPLE_WIDTH*(VIDEO_SAMPLE_HEIGHT / 2);//for Y, U and V
            decinputmediatype.SetUINT32(MFAttributesClsid.MF_MT_SAMPLE_SIZE, fixedSampleSize);
            decinputmediatype.SetUINT32(MFAttributesClsid.MF_MT_DEFAULT_STRIDE, VIDEO_SAMPLE_WIDTH);
            decinputmediatype.SetUINT32(MFAttributesClsid.MF_MT_FIXED_SIZE_SAMPLES, 1);
            decinputmediatype.SetUINT32(MFAttributesClsid.MF_MT_ALL_SAMPLES_INDEPENDENT, 1);
            MFExtern.MFSetAttributeRatio(decinputmediatype, MFAttributesClsid.MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

            hr =  decodertransform.SetInputType(0, decinputmediatype, 0);
            
            decodertransform.GetInputAvailableType(0, 0, out testdecinputmediatype);

            // setup media type for output of decoder 
            MFExtern.MFCreateMediaType(out decoutputmediatype);
            decoutputmediatype.SetGUID(MFAttributesClsid.MF_MT_MAJOR_TYPE, MFMediaType.Video);
            decoutputmediatype.SetGUID(MFAttributesClsid.MF_MT_SUBTYPE, MFMediaType.IYUV);
            MFExtern.MFSetAttributeSize(decoutputmediatype, MFAttributesClsid.MF_MT_FRAME_SIZE, VIDEO_SAMPLE_WIDTH, VIDEO_SAMPLE_HEIGHT);
            MFExtern.MFSetAttributeRatio(decoutputmediatype, MFAttributesClsid.MF_MT_FRAME_RATE, 30, 1);
            MFExtern.MFSetAttributeRatio(decoutputmediatype, MFAttributesClsid.MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
            decoutputmediatype.SetUINT32(MFAttributesClsid.MF_MT_INTERLACE_MODE,2);

            hr = decodertransform.SetOutputType(0, decoutputmediatype, 0);
            decodertransform.GetOutputAvailableType(0, 0, out testdecoutputmediatype);

            decodertransform.GetInputStatus(0, out mftStatus);
            if(mftStatus != MFTInputStatusFlags.AcceptData)
            {
                Debug.WriteLine("DECODER NOT ACCEPT INPUT DATA");
                return;
            }
            else
                Debug.WriteLine("PROCESS INPUT DONE>>>> " + mftStatus);

            decodertransform.ProcessMessage(MFTMessageType.CommandFlush,(IntPtr)null);
            decodertransform.ProcessMessage(MFTMessageType.NotifyBeginStreaming, (IntPtr)null);
            decodertransform.ProcessMessage(MFTMessageType.NotifyStartOfStream, (IntPtr)null);
        }


        

        public HResult DecodeSample(byte[] inbuffer, int insize, out byte[] outbuff, out int outsize)
        {
            // Process Input.
            MFTOutputDataBuffer outputDataBuffer;
            ProcessOutputStatus processOutputStatus = 0;
            IMFSample srcVideoSample;
            HResult mftProcessOutput;
            MFTOutputStreamInfo StreamInfo;
            IMFSample mftOutSample;
            IMFMediaBuffer pBuffer, srcBuffer; 


            Debug.WriteLine("Process sample...");
            outbuff = null;
            outsize = 0; 

            MFExtern.MFCreateSample(out srcVideoSample);
            MFExtern.MFCreateMemoryBuffer(insize, out srcBuffer);
            srcVideoSample.AddBuffer(srcBuffer);

            IntPtr inputdata = IntPtr.Zero;
            int reconBuffCurrLen = 0;
            int reconBuffMaxLen = 0;

            Debug.WriteLine("CONTINUE...1");

            // move input data to decoder
            srcBuffer.Lock(out inputdata, out reconBuffMaxLen, out reconBuffCurrLen);
            Marshal.Copy(inbuffer, 0, inputdata, insize);  
            srcBuffer.Unlock(); 
            
            srcBuffer.SetCurrentLength(insize);
             
            decodertransform.ProcessInput(0,srcVideoSample,0);
            
            decodertransform.GetOutputStreamInfo(0, out StreamInfo); 

            while (true)
			{
                MFExtern.MFCreateSample(out mftOutSample);
                MFExtern.MFCreateMemoryBuffer(StreamInfo.cbSize, out pBuffer);
                mftOutSample.AddBuffer(pBuffer);

                outputDataBuffer.dwStreamID = 0;
                outputDataBuffer.dwStatus = MFTOutputDataBufferFlags.None;
                outputDataBuffer.pEvents = null;
                outputDataBuffer.pSample = Marshal.GetIUnknownForObject(mftOutSample);

                MFTOutputDataBuffer[] odbs = { outputDataBuffer };
                 
                mftProcessOutput = decodertransform.ProcessOutput(MFTProcessOutputFlags.None, 1, odbs, out processOutputStatus);
                
                if(mftProcessOutput != HResult.MF_E_TRANSFORM_NEED_MORE_INPUT)
                {
                    IMFMediaBuffer buf;
                    int bufLength;
                    mftOutSample.ConvertToContiguousBuffer(out buf);
                    buf.GetCurrentLength(out bufLength);
                    
                    IntPtr poutputData = IntPtr.Zero; 
                    int buffCurrLen = 0;
                    int buffMaxLen = 0;

                    // get output data from decoder
                    buf.Lock(out poutputData, out buffMaxLen, out buffCurrLen);
                    outbuff = new byte[buffMaxLen];
                    Marshal.Copy(poutputData, outbuff, 0, buffCurrLen);
                    buf.Unlock();

                    outsize = buffCurrLen;
                    Debug.WriteLine("out put size: " + outsize + "\n");

                    Marshal.ReleaseComObject(buf);
                    Marshal.ReleaseComObject(mftOutSample); 
                }
                else
                {
                    break;
                }

                return mftProcessOutput;
            }

            return mftProcessOutput;
        }

        public void GetOutputRawImage()
        {

        }
    }
}
