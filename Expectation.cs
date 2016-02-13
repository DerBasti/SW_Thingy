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
    class Expectation
    {
        protected bool optionalFlag;
        public virtual bool test(Bitmap bmp) { return true; }
        public virtual void action(WindowDetection.Structure ws) { }

        public static Expectation current;

        public bool isOptional() { return optionalFlag; }

        static float addXOffset(float x)
        {
            return x + 0.11f;
        }
    }

    class ChoosingScreenExpectation : Expectation
    {

        public ChoosingScreenExpectation()
        {
            this.optionalFlag = false;
        }

        public override bool test(Bitmap bmp)
        {
            MouseInterface.PositionScope firstRow = new MouseInterface.PositionScope();
            firstRow.from.x = 0.68571f;
            firstRow.to.x = 0.875f;

            firstRow.from.y = firstRow.to.y = 0.6344f;

            Point from = PercentageF.ToCoordinate(bmp, firstRow.from);
            Point to = PercentageF.ToCoordinate(bmp, firstRow.to);


            MouseInterface.PositionScope secondRow = new MouseInterface.PositionScope();
            secondRow.from.x = 0.68571f;
            secondRow.to.x = 0.875f;

            secondRow.from.y = secondRow.to.y = 0.70161f;

            Point fromSecond = PercentageF.ToCoordinate(bmp, secondRow.from);
            Point toSecond = PercentageF.ToCoordinate(bmp, secondRow.to);

            float firstRowPercentage = WindowDetection.castRay(bmp, from, to, Color.FromArgb(255, 225, 195, 118));
            float secondRowPercentage = WindowDetection.castRay(bmp, fromSecond, toSecond, Color.FromArgb(255, 211, 165, 73));

            return (firstRowPercentage > 0.8f && secondRowPercentage > 0.9f);
        }

        public override void action(WindowDetection.Structure ws)
        {
            MouseInterface.PositionScope startButton = new MouseInterface.PositionScope();

            startButton.from.x = 0.71442f;
            startButton.from.y = 0.65831f;

            startButton.to.x = 0.85485f;
            startButton.to.y = 0.73668f;

            MouseInterface.randomizeClickInArea(ws, startButton);

            Expectation.current = new RunEndedExpectation();
        }
    }
    class RunEndedExpectation : Expectation
    {
        private DyingScreenExpectation dyingExpectation;
        private LevelingScreenExpectation successExpectation;
        private int expectationMet;

        public const int DYING_SCREEN_FLAG = 0x01;
        public const int SUCCESS_SCREEN_FLAG = 0x02;

        public RunEndedExpectation()
        {
            this.optionalFlag = false;
            expectationMet = 0;

            dyingExpectation = new DyingScreenExpectation();
            successExpectation = new LevelingScreenExpectation(false);
        }
        public override bool test(Bitmap bmp)
        {
            if (dyingExpectation.test(bmp))
            {
                expectationMet = DYING_SCREEN_FLAG;
            }
            else if (successExpectation.test(bmp))
            {
                expectationMet = SUCCESS_SCREEN_FLAG;
            }
            return expectationMet > 0;
        }

        public override void action(WindowDetection.Structure ws)
        {
            switch (expectationMet)
            {
                case DYING_SCREEN_FLAG:
                    dyingExpectation.action(ws);
                    Expectation.current = new LevelingScreenExpectation(true);
                    break;
                case SUCCESS_SCREEN_FLAG:
                    successExpectation.action(ws);
                    break;
            }
        }
    }
    class DyingScreenExpectation : Expectation
    {
        public static Color buttonBorderColor = Color.FromArgb(255, 163, 145, 89);
        public override bool test(Bitmap bmp)
        {
            MouseInterface.PositionScope ps = new MouseInterface.PositionScope();

            ps.from.x = 0.209375f;
            ps.to.x = 0.41484f;
            ps.from.y = ps.to.y = 0.70573f;

            float firstButton = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, ps.from),
                PercentageF.ToCoordinate(bmp, ps.to), buttonBorderColor);

            ps.from.x = 0.522656f;
            ps.to.x = 0.7203125f;

            float secondButton = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, ps.from),
                PercentageF.ToCoordinate(bmp, ps.to), buttonBorderColor);

            return firstButton > 0.8f && secondButton > 0.8f;
        }

        public override void action(WindowDetection.Structure ws)
        {
            MouseInterface.PositionScope ps = new MouseInterface.PositionScope();
            ps.from.x = 0.56641f;
            ps.to.x = 0.659375f;

            ps.from.y = 0.61458f;
            ps.to.y = 0.68f;

            MouseInterface.randomizeClickInArea(ws, ps);

            Expectation.current = new LevelingScreenExpectation(true);
        }
    }
    class LevelingScreenExpectation : Expectation
    {
        private DropScreenExpectation dropExpectation;
        private bool runFailedFlag;
        public LevelingScreenExpectation(bool didRunFail)
        {
            dropExpectation = new DropScreenExpectation();
            runFailedFlag = didRunFail;
        }
        public override bool test(Bitmap bmp)
        {
            MouseInterface.PositionScope currentRow = new MouseInterface.PositionScope();

            int foundMonsters = 0;
            for (int i = 0; i < 2; i++)
            {
                currentRow.from.y = currentRow.to.y = 0.67681f + (0.11f * i);
                for (int j = 0; j < 3; j++)
                {
                    currentRow.from.x = 0.206666f + (0.21913f * j);
                    currentRow.to.x = currentRow.from.x + 0.11f;

                    float levelPercentage = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, currentRow.from),
                        PercentageF.ToCoordinate(bmp, currentRow.to), Color.FromArgb(255, 208, 0, 19));

                    float barPercentage = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, currentRow.from),
                        PercentageF.ToCoordinate(bmp, currentRow.to), Color.FromArgb(255, 142, 113, 71));

                    if (levelPercentage >= 0.0f && barPercentage >= 0.0f && (barPercentage + levelPercentage) >= 0.9f)
                    {
                        foundMonsters++;
                    }
                }
            }
            return foundMonsters > 0;
        }

        public override void action(WindowDetection.Structure ws)
        {
            //Click in the middle of the window, twice
            MouseInterface.PositionScope ps = new MouseInterface.PositionScope();
            ps.from.x = ps.from.y = 0.45f;
            ps.to.x = ps.from.y = 0.55f;

            MouseInterface.randomizeClickInArea(ws, ps);

            Random r = new Random();
            int timeout = (int)1000 + (int)(r.NextDouble() * 400);
            System.Threading.Thread.Sleep(timeout);

            if (!runFailedFlag)
            {
                MouseInterface.randomizeClickInArea(ws, ps);
                timeout = (int)1000 + (int)(r.NextDouble() * 200);
                System.Threading.Thread.Sleep(timeout);
                Expectation.current = new DropScreenExpectation();
            }
            else
            {
                Expectation.current = new DecisionExpectation();
            }
        }
    }
    class DropScreenExpectation : Expectation
    {
        public static Color backgroundColor = Color.FromArgb(255, 37, 24, 15);
        public const int COLLECTABLE_DROP = 0x01;
        public const int SELLABLE_DROP = 0x02;
        private int buttonAmount;
        public override bool test(Bitmap bmp)
        {
            MouseInterface.PositionScope ps = new MouseInterface.PositionScope();
            ps.from.x = 0.26549f;
            ps.to.x = 0.66372f;

            ps.from.y = ps.to.y = 0.79087f;

            float buttonPercentage = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, ps.from),
                PercentageF.ToCoordinate(bmp, ps.to), Color.FromArgb(255, backgroundColor));

            //
            ps.from.y = ps.to.y = 0.29718f;
            float windowPercentage = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, ps.from),
                PercentageF.ToCoordinate(bmp, ps.to), Color.FromArgb(255, backgroundColor));


            buttonAmount = (int)(buttonPercentage / 0.33f);
            return buttonAmount > 0 && windowPercentage > 0.8f; //If previous things aligned
        }
        public override void action(WindowDetection.Structure ws)
        {
            MouseInterface.PositionScope ps = new MouseInterface.PositionScope();
            ps.from.y = 0.72901f;
            ps.to.y = 0.78056f;

            switch (buttonAmount)
            {
                case COLLECTABLE_DROP:
                    ps.from.x = 0.415929f;
                    ps.to.x = 0.50885f;
                    break;
                case SELLABLE_DROP:
                    ps.from.x = 0.323009f;
                    ps.to.x = 0.43363f;
                    break;
            }

            MouseInterface.randomizeClickInArea(ws, ps);

            //Give it time to sell
            System.Threading.Thread.Sleep(2000);

            Expectation.current = new DecisionExpectation();
        }
    }

    class DecisionExpectation : Expectation
    {
        public override bool test(Bitmap bmp)
        {
            MouseInterface.PositionScope replayButton = new MouseInterface.PositionScope();
            replayButton.from.x = 0.11891f;
            replayButton.to.x = 0.81009f;
            replayButton.from.y = replayButton.to.y = 0.48006f;

            float upperBorders = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, replayButton.from),
                PercentageF.ToCoordinate(bmp, replayButton.to), Color.FromArgb(255, 167, 144, 100));

            replayButton.from.y = replayButton.to.y = 0.60f;

            float lowerBorders = WindowDetection.castRay(bmp, PercentageF.ToCoordinate(bmp, replayButton.from),
                PercentageF.ToCoordinate(bmp, replayButton.to), Color.FromArgb(255, 164, 143, 95));

            return upperBorders > 0.8f && lowerBorders > 0.8f;
        }

        public override void action(WindowDetection.Structure ws)
        {
            MouseInterface.PositionScope replayButton = new MouseInterface.PositionScope();
            replayButton.from.x = 0.20102f;
            replayButton.from.y = 0.505698f;

            replayButton.to.x = 0.3524f;
            replayButton.to.y = 0.56267f;

            MouseInterface.randomizeClickInArea(ws, replayButton);
            System.Threading.Thread.Sleep(500);

            Expectation.current = new BuyEnergyExpectation();
        }
    }

    class BuyEnergyExpectation : Expectation
    {
        private bool isBuyingActive;
        public BuyEnergyExpectation()
        {
            optionalFlag = true;
            isBuyingActive = false;
        }
        public override bool test(Bitmap bmp)
        {
            return true;
        }

        public override void action(WindowDetection.Structure ws)
        {
            if (isBuyingActive)
            {
                //TODO
            }
            Expectation.current = new ChoosingScreenExpectation();
        }
    }
}
