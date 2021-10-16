using System;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace DicomViewer.UI.Controls
{
    public class PlotGraph : Grid
    {
        public BindingList<Point> Points
        {
            get { return (BindingList<Point>)GetValue(PointsProperty); }
            set { SetValue(PointsProperty, value); }
        }

        public static readonly DependencyProperty PointsProperty =
            DependencyProperty.Register("Points", typeof(BindingList<Point>), typeof(PlotGraph), new PropertyMetadata(null, OnPointsChanged));

        private static void OnPointsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not PlotGraph plot) return;
            plot.InvalidateVisual();

            var oldPts = e.OldValue as BindingList<Point>;
            var newPts = e.NewValue as BindingList<Point>;

            if (oldPts != null)
            {
                oldPts.ListChanged -= plot.OnPointsCollectionChanged;
            }

            if (newPts != null)
            {
                newPts.ListChanged += plot.OnPointsCollectionChanged;
            }
        }

        private void OnPointsCollectionChanged(object sender, ListChangedEventArgs e)
        {
            this.InvalidateVisual();
        }



        public double XTick
        {
            get { return (double)GetValue(XTickProperty); }
            set { SetValue(XTickProperty, value); }
        }

        public static readonly DependencyProperty XTickProperty =
            DependencyProperty.Register("XTick", typeof(double), typeof(PlotGraph), new PropertyMetadata(1.0d, OnTickChanged));

        public double YTick
        {
            get { return (double)GetValue(YTickProperty); }
            set { SetValue(YTickProperty, value); }
        }

        public static readonly DependencyProperty YTickProperty =
            DependencyProperty.Register("YTick", typeof(double), typeof(PlotGraph), new PropertyMetadata(1.0d, OnTickChanged));

        private static void OnTickChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not PlotGraph plot) return;
            plot.InvalidateVisual();
        }

        public Rect Bounds
        {
            get { return (Rect)GetValue(BoundsProperty); }
            set { SetValue(BoundsProperty, value); }
        }

        public static readonly DependencyProperty BoundsProperty =
            DependencyProperty.Register("Bounds", typeof(Rect), typeof(PlotGraph), new PropertyMetadata(new Rect(0,0,1,1), OnTickChanged));




        public int CursorPosition
        {
            get { return (int)GetValue(CursorPositionProperty); }
            set { SetValue(CursorPositionProperty, value); }
        }

        public static readonly DependencyProperty CursorPositionProperty =
            DependencyProperty.Register("CursorPosition", typeof(int), typeof(PlotGraph), new PropertyMetadata(0, OnTickChanged));


        Rect plotRect;
        double scaleX;
        double scaleY;
        Pen labelsPen;
        Pen linePen;
        protected override void OnRender(DrawingContext dc)
        {
            plotRect = GetPlotRect();
            scaleX = plotRect.Width / Bounds.Width;
            scaleY = plotRect.Height / Bounds.Height;
            if (labelsPen is null)
                labelsPen = new Pen(Brushes.Black, 1);
            if (linePen is null)
                linePen = new Pen(Brushes.IndianRed, 1);

            DrawAxes(dc);
            DrawTicksAndLabels2(dc);
            DrawTitles(dc);
            DrawPlot(dc);
            DrawCursor(dc);

            base.OnRender(dc);
        }

        private Rect GetPlotRect()
        {
            var plotWidth = ActualWidth * (1.0d - AxesMargin);
            var plotStartX = ActualWidth - plotWidth;
            var plotHeight = ActualHeight * (1.0d - AxesMargin);
            return new Rect(plotStartX, 0, plotWidth, plotHeight);
        }


        private void DrawPlot(DrawingContext dc)
        {
            if (Points is null || Points.Count == 0) return;

            var geometry = new StreamGeometry();
            using (var context = geometry.Open())
            {
                var tp = Transform(Points[0], plotRect.X, plotRect.Height, scaleX, scaleY);
                context.BeginFigure(tp, false, false);
                for (int i = 1; i < Points.Count; i++)
                    context.LineTo(
                        Transform(Points[i], plotRect.X, plotRect.Height, scaleX, scaleY),
                        true, false);
            }
            geometry.Freeze();

            //dc.PushTransform(new TranslateTransform(plotStartX, plotHeight));
            //dc.PushTransform(new ScaleTransform(scaleX, (-1) * scaleY));
            dc.DrawGeometry(null, linePen, geometry);
            //dc.Pop();
            //dc.Pop();

            //Point previous = new Point(plotStartX + scaleX * Points[0].X, plotHeight - scaleY * Points[0].Y);
            //for (int i = 0; i < Points.Count; i++)
            //{
            //    if (i > 0)
            //    {
            //        var current = new Point(plotStartX + scaleX * Points[i].X, plotHeight - scaleY * Points[i].Y);
            //        dc.DrawLine(pen, previous, current);
            //        previous = current;
            //    }
                    
            //}
        }

        private void DrawTitles(DrawingContext dc)
        {
            var yTitle = new FormattedText("Cross section area [mm2]", CultureInfo.CurrentCulture,
                FlowDirection.LeftToRight, new Typeface("Calibri"), 12, Brushes.Black, 1.25);
            var xpos1 = plotRect.X / 5 - yTitle.Width / 2;
            var ypos1 = plotRect.Height / 2 - yTitle.Height / 2;
            dc.PushTransform(new RotateTransform(-90, plotRect.X / 5, plotRect.Height / 2));
            dc.DrawText(yTitle, new Point(xpos1, ypos1));
            dc.Pop();


            var xTitle = new FormattedText("Cross section index", CultureInfo.CurrentCulture,
                FlowDirection.LeftToRight, new Typeface("Calibri"), 12, Brushes.Black, 1.25);
            var xpos2 = plotRect.X + plotRect.Width / 2 - xTitle.Width / 2;
            var ypos2 = ActualHeight * 0.98d;
            dc.DrawText(xTitle, new Point(xpos2, ypos2));
        }


        Point Transform(Point p, double x, double y, double scaleX, double scaleY)
        {
            return new Point(x + scaleX * p.X, y - scaleY * p.Y);
        }

        Point TransformBack(Point p, double x, double y, double scaleX, double scaleY)
        {
            return new Point((x - p.X) / scaleX, (y - p.Y) / scaleY);
        }

        [Obsolete]
        private void DrawTicksAndLabels(DrawingContext dc)
        {
            var pen = new Pen(Brushes.Black, 1);
            var plotWidth = ActualWidth * (1.0d - AxesMargin);
            var plotStartX = ActualWidth - plotWidth;
            var plotHeight = ActualHeight * (1.0d - AxesMargin);
            var scaleX = plotWidth / Bounds.Width;
            var scaleY = plotHeight / Bounds.Height;

            var xTicksCount = Bounds.Width / XTick; // except first and last
            var xStep = plotWidth / (xTicksCount + 1);
            var yTicksCount = Bounds.Height / YTick;
            var yStep = plotHeight / (yTicksCount + 1);

            var tickSize = ActualHeight * 0.01d;
            var previousX = plotStartX;
            for (int i = 0; i < xTicksCount; i++)
            {
                var newX = plotStartX + (i + 1) * xStep;
                if (Math.Abs(newX - previousX) <= 10) continue;

                dc.DrawLine(pen, new Point(newX, plotHeight - tickSize),
                    new Point(newX, plotHeight + tickSize));

                previousX = newX;

                var number = i * XTick;
                var label = new FormattedText(number.ToString(), CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                    new Typeface("Calibri"), 10, Brushes.Black);
                dc.DrawText(label, new Point(newX - label.Width / 2, plotHeight + 2 * tickSize));
            }

            var previousY = plotHeight;
            for (int i = 0; i < yTicksCount; i++)
            {
                var newY = plotHeight - (i + 1) * yStep;
                if (Math.Abs(newY - previousY) <= 10) continue;

                dc.DrawLine(pen, new Point(plotStartX - tickSize, newY),
                    new Point(plotStartX + tickSize, newY));

                previousY = newY;

                var number = i * YTick;
                var label = new FormattedText(number.ToString(), CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                    new Typeface("Calibri"), 10, Brushes.Black);
                dc.DrawText(label, new Point(plotStartX - 2 * tickSize - label.Width, newY - label.Height / 2));
            }
        }

        private void DrawTicksAndLabels2(DrawingContext dc)
        {
            var tickSize = ActualHeight * 0.01d;
            double x = XTick;
            while (x < Bounds.Width)
            {
                var tp = Transform(new Point(x, 0), plotRect.X, plotRect.Height, scaleX, scaleY);
                dc.DrawLine(labelsPen, new Point(tp.X, plotRect.Height - tickSize), new Point(tp.X, plotRect.Height + tickSize));

                var label = new FormattedText(x.ToString(), CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                    new Typeface("Calibri"), 9, Brushes.Black, 1.25);
                dc.DrawText(label, new Point(tp.X - label.Width / 2, plotRect.Height + 2 * tickSize));
                x += XTick;
            }

            double y = YTick;
            while (y < Bounds.Height)
            {
                var tp = Transform(new Point(0, y), plotRect.X, plotRect.Height, scaleX, scaleY);
                dc.DrawLine(labelsPen, new Point(plotRect.X - tickSize, tp.Y),
                    new Point(plotRect.X + tickSize, tp.Y));


                var label = new FormattedText(y.ToString(), CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                  new Typeface("Calibri"), 9, Brushes.Black, 1.25);
                dc.DrawText(label, new Point(plotRect.X - 2 * tickSize - label.Width, tp.Y - label.Height / 2));
                y += YTick;
            }
        }
        
        const double AxesMargin = 0.1d;
        private void DrawAxes(DrawingContext dc)
        {
            var xAxis = plotRect.Height;
            var yAxis = plotRect.X;
            var arrowWidth = yAxis / 5;
            var arrowHeight = yAxis / 6;

            dc.DrawLine(labelsPen, new Point(yAxis, xAxis), new Point(ActualWidth, xAxis));
            dc.DrawLine(labelsPen, new Point(ActualWidth, xAxis), new Point(ActualWidth - arrowHeight / 2, xAxis - arrowWidth / 2));
            dc.DrawLine(labelsPen, new Point(ActualWidth, xAxis), new Point(ActualWidth - arrowHeight / 2, xAxis + arrowWidth / 2));
                        
            dc.DrawLine(labelsPen, new Point(yAxis, 0), new Point(yAxis, xAxis));
            dc.DrawLine(labelsPen, new Point(yAxis, 0), new Point(yAxis - arrowWidth / 2, arrowHeight / 2));
            dc.DrawLine(labelsPen, new Point(yAxis, 0), new Point(yAxis + arrowWidth / 2, arrowHeight / 2));
        }

        private void DrawCursor(DrawingContext dc)
        {
            if (Points is null || Points.Count == 0 || CursorPosition >= Points.Count) return;

            if (cursorPen is null)
                cursorPen = new Pen(Brushes.LightGray, 1);

            var pt = Points[CursorPosition];
            var tp = Transform(pt, plotRect.X, plotRect.Height, scaleX, scaleY);
            dc.DrawLine(cursorPen, new Point(tp.X, 0), new Point(tp.X, plotRect.Height));
            dc.DrawLine(cursorPen, new Point(plotRect.X, tp.Y), new Point(ActualWidth, tp.Y));
        }
        Pen cursorPen;

        public Brush LineBrush
        {
            get { return (Brush)GetValue(LineBrushProperty); }
            set { SetValue(LineBrushProperty, value); }
        }

        public static readonly DependencyProperty LineBrushProperty =
            DependencyProperty.Register("LineBrush", typeof(Brush), typeof(PlotGraph), new PropertyMetadata(Brushes.IndianRed));

    }
}
