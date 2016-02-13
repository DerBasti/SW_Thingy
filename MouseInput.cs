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
    
        [StructLayout(LayoutKind.Sequential)]
        public struct POINT
        {
            public int X;
            public int Y;

            public static implicit operator Point(POINT point)
            {
                return new Point(point.X, point.Y);
            }
        }

        /// <summary>
        /// Retrieves the cursor's position, in screen coordinates.
        /// </summary>
        /// <see>See MSDN documentation for further information.</see>
        /// 
        class MouseInterface
        {
            [DllImport("user32.dll")]
            public static extern bool GetCursorPos(out POINT lpPoint);

            [DllImport("user32.dll")]
            public static extern bool SetCursorPos(int x, int y);
           
            [DllImport("user32.dll")]
            public static extern IntPtr SetCapture(IntPtr hWnd);

            [DllImport("user32.dll")]
            public static extern long ReleaseCapture();

            [DllImport("user32.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
            public static extern void mouse_event(MouseEventFlags dwFlags, uint dx, uint dy, uint cButtons, UIntPtr dwExtraInfo);
            public const int WM_LBUTTONDOWN = 0x201;
            public const int WM_LBUTTONUP = 0x202;

            [Flags]
            public enum MouseEventFlags : uint
            {
                MOUSEEVENTF_MOVE = 0x0001,
                MOUSEEVENTF_LEFTDOWN = 0x0002,
                MOUSEEVENTF_LEFTUP = 0x0004,
                MOUSEEVENTF_RIGHTDOWN = 0x0008,
                MOUSEEVENTF_RIGHTUP = 0x0010,
                MOUSEEVENTF_MIDDLEDOWN = 0x0020,
                MOUSEEVENTF_MIDDLEUP = 0x0040,
                MOUSEEVENTF_XDOWN = 0x0080,
                MOUSEEVENTF_XUP = 0x0100,
                MOUSEEVENTF_WHEEL = 0x0800,
                MOUSEEVENTF_VIRTUALDESK = 0x4000,
                MOUSEEVENTF_ABSOLUTE = 0x8000
            }

            public class PositionScope
            {
                public PercentageF from;
                public PercentageF to;

                public PositionScope()
                {
                    from = new PercentageF();
                    to = new PercentageF();
                }
            }

            private static IntPtr makeLParam(Point p)
            {
                IntPtr result = IntPtr.Add(IntPtr.Zero, p.X);
                result = IntPtr.Add(result, p.Y << 16);
                return result;
            }

            private static Point prepareClickPoint(WindowDetection.Structure ws, Point position, bool exact)
            {
                int clickPositionX = position.X;
                int clickPositionY = position.Y;

                if (!exact)
                {
                    Random r = new Random();
                    clickPositionX += (int)(r.NextDouble() * 20);
                    clickPositionY += (int)(r.NextDouble() * 10);
                }
                return new Point(clickPositionX, clickPositionY);
            }

            public static void simulateClick(WindowDetection.Structure ws, Point position)
            {
                simulateClick(ws, position, false);
            }
            public static void simulateClick(WindowDetection.Structure ws, Point position, bool exactFlag)
            {
                Point clickPoint = prepareClickPoint(ws, position, exactFlag);

                IntPtr captureWindow = SetCapture(ws.contentWindow);
                WindowDetection.PostMessage(ws.contentWindow, WM_LBUTTONDOWN, IntPtr.Add(IntPtr.Zero, 1), makeLParam(clickPoint));
                WindowDetection.PostMessage(ws.contentWindow, WM_LBUTTONUP, IntPtr.Zero, makeLParam(clickPoint));
                ReleaseCapture();
                SetCapture(captureWindow);
            }

            public static void simulateClickMechanically(WindowDetection.Structure ws, Point position, bool exact)
            {
                IntPtr previousTopWindow = WindowDetection.GetForegroundWindow();
                WindowDetection.SetForegroundWindow(ws.mainWindow);
                WindowDetection.SetFocus(ws.mainWindow);
                System.Threading.Thread.Sleep(25);

                POINT p = new POINT();
                GetCursorPos(out p);

                position.X += ws.position.Left;
                position.Y += ws.position.Top;

                Point clickPos = prepareClickPoint(ws, position, exact);

                SetCursorPos(clickPos.X, clickPos.Y);
                System.Threading.Thread.Sleep(1);

                mouse_event(MouseEventFlags.MOUSEEVENTF_LEFTDOWN, (uint)clickPos.X, (uint)clickPos.Y, (uint)0, UIntPtr.Zero);
                mouse_event(MouseEventFlags.MOUSEEVENTF_LEFTUP, (uint)clickPos.X, (uint)clickPos.Y, 0, UIntPtr.Zero);

                SetCursorPos(p.X, p.Y);

                WindowDetection.SetForegroundWindow(previousTopWindow);
                WindowDetection.SetFocus(previousTopWindow);
            }

            public static void randomizeClickInArea(WindowDetection.Structure ws, PositionScope scope)
            {
                int basePixelX = (int)(scope.from.x * ws.currentFrame.Width);
                int diffAmountX = (int)((scope.to.x - scope.from.x) * ws.currentFrame.Width);

                int basePixelY = (int)(scope.from.y * ws.currentFrame.Height);
                int diffAmountY = (int)((scope.to.y - scope.from.y) * ws.currentFrame.Height);

                Random r = new Random();
                Point p = new Point((int)(diffAmountX * r.NextDouble()) + basePixelX,
                    (int)(diffAmountY * r.NextDouble()) + basePixelY);

                simulateClick(ws, p, true);
            }
        }
}