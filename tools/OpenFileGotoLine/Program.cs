using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using EnvDTE;
using EnvDTE80;
using System.Threading;
using System.Runtime.InteropServices.ComTypes;
//using VSLangProj;

namespace OpenFileGotoLine
{
    static class Program
    {

        //调用exe
        [DllImport("kernel32.dll")]
        public static extern int WinExec(string exeName, int operType);

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static int Main(string [] args)
        {
            try
            {
                if(args.Length<2)
                {
                    return -1;
                }

                String filename = args[0];
                int fileline;
                int.TryParse(args[1], out fileline);
                
                //filename = "d:\\t.cpp";
                //fileline = 12;

                /*
                string exe = @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe";
                string cmd= exe+@" /edit "+filename;
                WinExec(cmd, 5);
                */

                //前提:VS必须已运行
                EnvDTE80.DTE2 dte2 = null;
                dte2 = (EnvDTE80.DTE2)Marshal.GetActiveObject("VisualStudio.DTE");
                if(dte2!=null)
                {
                    dte2.MainWindow.Activate();
                    EnvDTE.Window w = dte2.ItemOperations.OpenFile(filename);
                    ((EnvDTE.TextSelection)dte2.ActiveDocument.Selection).GotoLine(fileline);
                    return 0;
                }
            }
            catch (Exception e)
            {
                Console.Write(e.Message);
            }

            return -1;

            /*
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
            //*/
        }
    }
}
