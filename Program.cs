using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace SW_Thingy
{

    public class PercentageF
    {
        public float x;
        public float y;

        public static Point ToCoordinate(Bitmap bmp, PercentageF f)
        {
            return new Point((int)(bmp.Width * f.x), (int)(bmp.Height * f.y));
        }
    }

    class Program
    {

        class CycleState
        {
            private WindowDetection.Structure ws;

            public CycleState(WindowDetection.Structure ws)
            {
                this.ws = ws;
                Expectation.current = new ChoosingScreenExpectation();
            }

            public bool isCurrentCycleValid()
            {
                return Expectation.current.test(ws.currentFrame);
            }

            public bool isCurrentCycleOptional()
            {
                return Expectation.current.isOptional();
            }

            public void fireAction()
            {
                Expectation.current.action(ws);
            }

            public virtual String getCycleName()
            {
                return Expectation.current.GetType().Name;
            }

        }


        private static byte GetCurrentState(CycleState gameState, Bitmap bmp)
        {
            bool validityFlag = gameState.isCurrentCycleValid();
            if (!validityFlag && !gameState.isCurrentCycleOptional())
            {
                return 0;
            }
            //Even though an optional step may occur, it can get skipped this way
            if (validityFlag)
            {
                Console.WriteLine("Validity of State \"{0}\" was successfully evaluated.", gameState.getCycleName());
                gameState.fireAction();
                Console.WriteLine("Next step: \"{0}\"", gameState.getCycleName());
            }
            return 1;
        }
        static void Main(string[] args)
        {
            WindowDetection.Structure ws = WindowDetection.GetRecordingWindow("Mobizen");
            CycleState gameState = new CycleState(ws);
            Random r = new Random();
            while (ws.contentWindow != IntPtr.Zero)
            {
                WindowDetection.GetImageFromWindow(ws);
                ws.currentFrame.Save("D:\\bmp_output.bmp");
                byte state = GetCurrentState(gameState, ws.currentFrame);
                int timeout = 500 + (int)(r.NextDouble() * 400);

                Console.WriteLine("Idling for {0}ms....", timeout);
                System.Threading.Thread.Sleep(timeout);
            }
        }
    }
}
