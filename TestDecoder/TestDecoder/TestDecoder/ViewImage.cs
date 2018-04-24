using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

using MediaFoundation;
using TestDecoder;

namespace TestDecoder
{
    public partial class ViewImage : Form
    {
        IMFDecoder decoder = new IMFDecoder();
        public ViewImage()
        {
            InitializeComponent();

            string inputfolder = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/captureframe/";
            string outputyuvfolder = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/yuvimg/";
            string outputimgfolder = "D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/imgjpeg/";
            int count = 0;
            string inputname = "";
            string outputname = "";
            byte[] framefile = null;
            
            int outlen = 0; 

            while(count < 160)
            {
                byte[] outdata = null;

                inputname = inputfolder + count + "_rtsp_capture.endc";
                outputname = outputyuvfolder + count + "_yuvimg.yuv";

                if(!File.Exists(inputname))
                {
                    Debug.WriteLine("File is not exist");
                    return;
                }
                framefile = File.ReadAllBytes(inputname);
                Debug.WriteLine("File " + inputname + " size: " + framefile.Length + "\n");
                
                if (decoder.DecodeSample(framefile, framefile.Length, out outdata, out outlen) != HResult.MF_E_TRANSFORM_NEED_MORE_INPUT)
                {
                    Debug.WriteLine("WRite to file: " + outputname + " leng: " + outlen + "\n" );
                    if(!File.Exists(outputname))
                    { 
                        WriteFileYUV(outdata, outputname, outlen);
                        ConvertYUV2JEPG(outputname, outputimgfolder + count + "_imgyuv.jpeg");
                    } 
                } 
                count++;
            }

        }
        private void WriteFileYUV(byte[] data, string filename, int len)
        {
            // write a frame to file within the header
            FileStream filewriter;

            filewriter = new FileStream(filename, FileMode.Create);

            filewriter.Write(data, 0, len);

            filewriter.Flush(true);
        }

        private void ConvertYUV2JEPG(string src, string dst)
        {

            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = "cmd.exe";
            startInfo.Arguments = "/C D:/Project/IPCamera/IPCamera/TestDecoder/TestDecoder/output/ffmpeg.exe -vcodec rawvideo -s 1920x1080 -pix_fmt yuv420p -i " + src + " -vframes 1 " + dst +" -y";
            process.StartInfo = startInfo;
            process.Start();
        }
    }
}
