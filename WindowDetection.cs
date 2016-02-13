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
    class WindowDetection
    {
        // Delegate to filter which windows to include 
        private delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern bool EnumWindows(EnumWindowsProc enumProc, IntPtr lParam);


        [DllImport("user32.dll")]
        private static extern bool EnumChildWindows(IntPtr parent, EnumWindowsProc enumProc, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern IntPtr GetWindow(IntPtr wnd, uint flag);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool GetWindowRect(IntPtr hWnd, ref RECT lpRect);

        [DllImport("user32.dll")]
        public static extern Int32 PostMessage(IntPtr hWnd, Int32 wMsg, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr SetFocus(IntPtr hWnd);

        [DllImport("user32.dll")]
        public static extern bool SetForegroundWindow(IntPtr hWnd);

        [DllImport("user32.dll")]
        public static extern IntPtr GetForegroundWindow();



        [System.Runtime.InteropServices.DllImportAttribute("gdi32.dll")]
        private static extern int BitBlt(
          IntPtr hdcDest,     // handle to destination DC (device context)
          int nXDest,         // x-coord of destination upper-left corner
          int nYDest,         // y-coord of destination upper-left corner
          int nWidth,         // width of destination rectangle
          int nHeight,        // height of destination rectangle
          IntPtr hdcSrc,      // handle to source DC
          int nXSrc,          // x-coordinate of source upper-left corner
          int nYSrc,          // y-coordinate of source upper-left corner
          System.Int32 dwRop  // raster operation code
          );

        [StructLayout(LayoutKind.Sequential)]
        public struct RECT
        {
            public int Left;        // x position of upper-left corner
            public int Top;         // y position of upper-left corner
            public int Right;       // x position of lower-right corner
            public int Bottom;      // y position of lower-right corner
        }

        public class Structure
        {
            public IntPtr contentWindow;
            public IntPtr messageWindow;
            public IntPtr mainWindow;
            public Bitmap currentFrame;
            public RECT position;
        }

        const int GWL_STYLE = -16;
        const long WS_VISIBLE = 0x10000000L;

        public static Structure GetRecordingWindow(String name)
        {
            Process[] processes = Process.GetProcessesByName("Mobizen");
            Structure ws = new Structure();
            foreach (Process p in processes)
            {
                Point position = new Point(400, 300);
                ws.mainWindow = p.MainWindowHandle;
                EnumWindows(delegate(IntPtr wnd, IntPtr param)
                {
                    if (GetWindow(wnd, 4) == ws.mainWindow)
                    {
                        EnumWindowsProc wcp = delegate(IntPtr w, IntPtr x)
                        {
                            return true;
                        };
                        if (EnumChildWindows(wnd, wcp, IntPtr.Zero))
                        {
                            ws.messageWindow = wnd;
                            return false;
                        }
                    }

                    return true;
                }, IntPtr.Zero);

                EnumChildWindows(ws.messageWindow, delegate(IntPtr wnd, IntPtr param)
                {
                    if (ws.contentWindow == null)
                    {
                        ws.contentWindow = wnd;
                        ws.position = new RECT();
                        GetWindowRect(wnd, ref ws.position);
                        return true;
                    }
                    RECT current = new RECT();
                    GetWindowRect(wnd, ref current);

                    int bestTotal = (ws.position.Right - ws.position.Left) * (ws.position.Bottom - ws.position.Top);
                    int currentTotal = (current.Right - current.Left) * (current.Bottom - current.Top);

                    int windowWidth = current.Right - current.Left;
                    int windowHeight = current.Bottom - current.Top;

                    if (currentTotal > bestTotal && windowWidth > windowHeight && (GetWindowLong(wnd, GWL_STYLE) & WS_VISIBLE) > 0)
                    {
                        ws.contentWindow = wnd;
                        ws.position = current;
                    }
                    return true;
                }, IntPtr.Zero);
            }
            return ws;
        }

        class WindowHierarchy
        {
            public IntPtr window;
            public List<WindowHierarchy> subwindows;
            public int flag;

            public const int OWNED = 0x01;
            public const int CHILD = 0x02;
            public WindowHierarchy(IntPtr windowHandle, int flag)
            {
                this.window = windowHandle;
                this.flag = flag;
                this.subwindows = new List<WindowHierarchy>();
                EnumWindows(delegate(IntPtr wnd, IntPtr param)
                {
                    //Get the Window where the owner == this window
                    if (GetWindow(wnd, 4) == param)
                    {
                        EnumWindowsProc wcp = delegate(IntPtr w, IntPtr x)
                        {
                            return true;
                        };
                        subwindows.Add(new WindowHierarchy(wnd, WindowHierarchy.OWNED));
                    }
                    return true;
                }, windowHandle);

                EnumChildWindows(windowHandle, delegate(IntPtr hwnd, IntPtr lParam)
                {
                    subwindows.Add(new WindowHierarchy(hwnd, WindowHierarchy.CHILD));
                    return true;
                }, IntPtr.Zero);
            }
        }

        public static void GetImageFromWindow(Structure ws)
        {
            using (Graphics g = Graphics.FromHwnd(ws.contentWindow))
            {
                GetWindowRect(ws.contentWindow, ref ws.position);
                ws.currentFrame = new Bitmap(ws.position.Right - ws.position.Left, ws.position.Bottom - ws.position.Top);
                using (Graphics image = Graphics.FromImage(ws.currentFrame))
                {
                    IntPtr hdc = image.GetHdc();
                    IntPtr srcDC = g.GetHdc();
                    BitBlt(hdc, 0, 0, ws.currentFrame.Width, ws.currentFrame.Height, srcDC, 0, 0, 0xCC0020);
                    image.ReleaseHdc();
                }
            }
        }
        public static float castRay(Bitmap bmp, Point from, Point to, Color comparisonColor)
        {
            int pixelOverlap = 0;
            for (int x = from.X; x <= to.X; x++)
            {
                for (int y = from.Y; y <= to.Y; y++)
                {
                    Color c = bmp.GetPixel(x, y);
                    int colorDiff = Math.Abs(c.R - comparisonColor.R) + Math.Abs(c.G - comparisonColor.G) +
                        Math.Abs(c.B - comparisonColor.B);
                    float percentage = colorDiff / 255.0f;
                    if (percentage < 0.2f)
                    {
                        pixelOverlap++;
                    }
                }
            }
            int xDiff = (to.X - from.X) + 1;
            int yDiff = (to.Y - from.Y) + 1;
            return (pixelOverlap) / (float)(xDiff * yDiff);
        }
        /*
        static void printSubTree(WindowHierarchy wnd, RECT r)
        {
            StringBuilder sb = new StringBuilder();
            GetWindowText(wnd.window, sb, 255);
            Console.WriteLine("[{0}] {1} - FLAG: {2}", wnd.window, sb.ToString(), wnd.flag);
            if (!tracker.Contains(wnd.window))
            {
                tracker.Add(wnd.window);
                int rng = (r.Right - r.Left) / 2 | ((r.Bottom - r.Top) / 2) << 16;
                rng += 0x300030;
                SendMessage(wnd.window, 0x201, 0x01, rng);
                System.Threading.Thread.Sleep(100);
                SendMessage(wnd.window, 0x202, 0x00, rng);
                System.Threading.Thread.Sleep(500);
            }
            for (int i = 0; i < wnd.subwindows.Count; i++)
            {
                printSubTree(wnd.subwindows[i], r);
            }
        }
        static WindowHierarchy findWindow(WindowHierarchy wnd, String title)
        {
            StringBuilder sb = new StringBuilder();
            GetWindowText(wnd.window, sb, 255);
            if (sb.ToString().Contains(title))
            {
                return wnd;
            }

            WindowHierarchy result = null;
            for (int i = 0; i < wnd.subwindows.Count; i++)
            {
                result = findWindow(wnd.subwindows[i], title);
                if (result != null)
                {
                    Console.WriteLine("Found window with title: {0}", title);
                    break;
                }
            }
            return result;
        }
        */
    }
}
