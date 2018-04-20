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
            IntPtr pDll = NativeMethods.LoadLibrary(@"D:\Project\IPCamera\IPCamera\TestDecoder\TestDecoder\Debug\IMFClassMiddle.dll");

            
            //oh dear, error handling here
            if (pDll == IntPtr.Zero)
            {
                Debug.WriteLine("ERRORR pDll >>>>>>>>>>>");
                return; 
            }

            IntPtr pAddressOfFunctionToCall = NativeMethods.GetProcAddress(pDll, "MultiplyByTen");
            //oh dear, error handling here
            if (pAddressOfFunctionToCall == IntPtr.Zero)
            {
                Debug.WriteLine("ERRORR pAddressOfFunctionToCall >>>>>>>>>");
                return;
            }



            MultiplyByTen multiplyByTen = (MultiplyByTen)Marshal.GetDelegateForFunctionPointer(
                                                                                    pAddressOfFunctionToCall,
                                                                                    typeof(MultiplyByTen));

            int theResult = multiplyByTen(300);
            //remaining code here
            Console.WriteLine(theResult); 

            bool result = NativeMethods.FreeLibrary(pDll); 



            IntPtr module = NativeMethods.LoadLibrary("msmpeg2vdec.dll");

            IntPtr myFunction = NativeMethods.GetProcAddress(module, "ICodecAPI");



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
