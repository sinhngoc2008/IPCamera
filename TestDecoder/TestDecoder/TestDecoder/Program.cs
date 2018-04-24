using System;
using System.Collections.Generic;
using System.Linq; 
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;



namespace TestDecoder
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int MultiplyByTen(int numberToMultiply);
        private delegate IntPtr ICodecAPI();

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false); 
            Application.Run(new ViewImage());



        }
    }
    

    static class NativeMethods
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr LoadLibrary(string dllToLoad);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);

         
        [DllImport("kernel32.dll")]
        public static extern bool FreeLibrary(IntPtr hModule);

        
        [DllImport("msmpeg2vdec.dll")]
        public static extern IntPtr ICodecAPI();

        [DllImport("msmpeg2vdec.dll")]
        static public extern IntPtr IMFTransform();
    }

}
