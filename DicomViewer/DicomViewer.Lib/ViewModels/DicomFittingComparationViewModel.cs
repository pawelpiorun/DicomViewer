using DicomViewer.Lib.Algorithms;
using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using DicomViewer.Lib.Native.Dicom;
using DicomViewer.Lib.Threading;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Input;
using System.Windows.Media.Media3D;

namespace DicomViewer.Lib.ViewModels
{
    public class DicomFittingComparationViewModel : NotifyPropertyChanged
    {
        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr DicomProcessing_GetTransform(double[] srcPoints, double[] srcCenter, double[] trgPoints, double[] trgCenter);

        private string firstPath;
        private string secondPath;
        private string resultsPath;

        public DicomFittingComparationViewModel(string firstPath, string secondPath, string resultsPath)
        {
            this.firstPath = firstPath;
            this.secondPath = secondPath;
            this.resultsPath = resultsPath;
        }

        public void Load(Action onDone)
        {
            LoadFirstDicom(firstPath, () => LoadSecondDicom(secondPath, () =>
            {
                onDone();
                GetTransformInfo(resultsPath);
            }));
        }

        private void LoadSecondDicom(string secondPath, Action onDone)
        {
            if (secondDicom != null)
            {
                secondDicom.Cleanup();
                secondDicom = null;
            }
            secondDicom = new DicomSeries();

            var progressViewModel = new ProgressViewModel()
            {
                SubTitle = "Reading DICOM series...",
                Message = secondPath
            };
            progressViewModel.Show();

            secondDicom.Open(secondPath,
                async: true,
                onProgress: progressViewModel.SetProgress,
                onDone: () =>
                {
                    var algorithm = new MandibularGeneralThresholdAlgorithm();
                    algorithm.Run(secondDicom, true, onDone: () =>
                        {
                            Threads.UI(() =>
                            {
                                progressViewModel.Close();
                                OnPropertyChanged(nameof(IsDataLoaded));
                                OnPropertyChanged(nameof(FirstDicom));
                                OnPropertyChanged(nameof(SecondDicom));
                                onDone();
                                FirstDicom.RequestRender();
                                SecondDicom.RequestRender();
                                Presets.SelectPreset(PresetType.Mandibular);
                            });
                        });
                });
        }

        private void LoadFirstDicom(string firstPath, Action onDone)
        {
            if (firstDicom != null)
            {
                firstDicom.Cleanup();
                firstDicom = null;
            }
            firstDicom = new DicomSeries();

            var progressViewModel = new ProgressViewModel()
            {
                SubTitle = "Reading DICOM series...",
                Message = firstPath
            };
            progressViewModel.Show();

            firstDicom.Open(firstPath,
                async: true,
                onProgress: progressViewModel.SetProgress,
                onDone: () =>
                {
                    Threads.UI(() =>
                    {
                        var algorithm = new MandibularGeneralThresholdAlgorithm();
                        algorithm.Run(firstDicom, true);

                        progressViewModel.Close();
                        onDone?.Invoke();
                        OnPropertyChanged(nameof(IsDataLoaded));
                    });
                });
        }

        private void GetTransformInfo(string resultsPath)
        {
            var csvLines = File.ReadAllLines(resultsPath);
            var props = csvLines.First().Split(';').ToList();
            var firstProps = csvLines.FirstOrDefault(x => x.Contains(firstDicom.FullPath)).Split(';');
            var secondProps = csvLines.FirstOrDefault(x => x.Contains(secondDicom.FullPath)).Split(';');
            var trgPoints = GetSkullPoints(props, firstProps);
            var trgCenter = GetSkullCenter(props, firstProps);
            additionalSourcePoints = GetSkullPoints(props, secondProps);
            var srcCenter = GetSkullCenter(props, secondProps);

            var trPtr = DicomProcessing_GetTransform(additionalSourcePoints, srcCenter, trgPoints, trgCenter);
            var trArr = new double[16];
            Marshal.Copy(trPtr, trArr, 0, 16);

            transform = new MatrixTransform3D(
                new Matrix3D(
                trArr[0], trArr[1], trArr[2], trArr[3],
                trArr[4], trArr[5], trArr[6], trArr[7],
                trArr[8], trArr[9], trArr[10], trArr[11],
                trArr[12], trArr[13], trArr[14], trArr[15]
                ));

            OnPropertyChanged(nameof(Transform));

            SelectedPoints = new BindingList<Point3D>
            {
                new Point3D { X = trgPoints[0], Y = trgPoints[1], Z = trgPoints[2] },
                new Point3D { X = trgPoints[3], Y = trgPoints[4], Z = trgPoints[5] },
                new Point3D { X = trgPoints[6], Y = trgPoints[7], Z = trgPoints[8] }
            };

            UpdateAdditionalSelectedPoints();

            for (int i = 0; i < SelectedPoints.Count; i++)
            {
                var pt1 = SelectedPoints[i];
                var pt2 = AdditionalSelectedPoints[i];
                var dst = Math.Sqrt(Math.Pow(pt1.X - pt2.X, 2) + Math.Pow(pt1.Y - pt2.Y, 2) + Math.Pow(pt1.Z - pt2.Z, 2));
                Console.WriteLine(i + ": " + dst);
            }

            UpdateTransformElements();
        }

        private void UpdateAdditionalSelectedPoints()
        {
            var mtx = transform.Matrix;
            AdditionalSelectedPoints = new BindingList<Point3D>
            {
                new Point3D
                {
                    X = additionalSourcePoints[0] * mtx.M11 + additionalSourcePoints[1] * mtx.M12 + additionalSourcePoints[2] * mtx.M13 + mtx.M14,
                    Y = additionalSourcePoints[0] * mtx.M21 + additionalSourcePoints[1] * mtx.M22 + additionalSourcePoints[2] * mtx.M23 + mtx.M24,
                    Z = additionalSourcePoints[0] * mtx.M31 + additionalSourcePoints[1] * mtx.M32 + additionalSourcePoints[2] * mtx.M33 + mtx.M34,
                },
                new Point3D
                {
                    X = additionalSourcePoints[3] * mtx.M11 + additionalSourcePoints[4] * mtx.M12 + additionalSourcePoints[5] * mtx.M13 + mtx.M14,
                    Y = additionalSourcePoints[3] * mtx.M21 + additionalSourcePoints[4] * mtx.M22 + additionalSourcePoints[5] * mtx.M23 + mtx.M24,
                    Z = additionalSourcePoints[3] * mtx.M31 + additionalSourcePoints[4] * mtx.M32 + additionalSourcePoints[5] * mtx.M33 + mtx.M34,
                },
                new Point3D
                {
                    X = additionalSourcePoints[6] * mtx.M11 + additionalSourcePoints[7] * mtx.M12 + additionalSourcePoints[8] * mtx.M13 + mtx.M14,
                    Y = additionalSourcePoints[6] * mtx.M21 + additionalSourcePoints[7] * mtx.M22 + additionalSourcePoints[8] * mtx.M23 + mtx.M24,
                    Z = additionalSourcePoints[6] * mtx.M31 + additionalSourcePoints[7] * mtx.M32 + additionalSourcePoints[8] * mtx.M33 + mtx.M34,
                },
            };
        }

        private double[] GetSkullPoints(List<string> props, string[] imageProps)
        {
            var originX = GetDoubleProp(props, imageProps, Mandibular.OriginXTag);
            var originY = GetDoubleProp(props, imageProps, Mandibular.OriginYTag);
            var originZ = GetDoubleProp(props, imageProps, Mandibular.OriginZTag);
            var spacingX = GetDoubleProp(props, imageProps, Mandibular.SpacingXTag);
            var spacingY = GetDoubleProp(props, imageProps, Mandibular.SpacingYTag);
            var spacingZ = GetDoubleProp(props, imageProps, Mandibular.SpacingZTag);

            var leftX = GetDoubleProp(props, imageProps, Mandibular.LeftZygomaticXTag) * spacingX + originX;
            var leftY = GetDoubleProp(props, imageProps, Mandibular.LeftZygomaticYTag) * spacingY + originY;
            var leftZ = GetDoubleProp(props, imageProps, Mandibular.LeftZygomaticZTag) * spacingZ + originZ;
            var rightX = GetDoubleProp(props, imageProps, Mandibular.RightZygomaticXTag) * spacingX + originX;
            var rightY = GetDoubleProp(props, imageProps, Mandibular.RightZygomaticYTag) * spacingY + originY;
            var rightZ = GetDoubleProp(props, imageProps, Mandibular.RightZygomaticZTag) * spacingZ + originZ;
            var nasionX = GetDoubleProp(props, imageProps, Mandibular.NasionXTag) * spacingX + originX;
            var nasionY = GetDoubleProp(props, imageProps, Mandibular.NasionYTag) * spacingY + originY;
            var nasionZ = GetDoubleProp(props, imageProps, Mandibular.NasionZTag) * spacingZ + originZ;

            return new double[]
            {
                leftX, leftY, leftZ,
                rightX, rightY, rightZ,
                nasionX, nasionY, nasionZ
            };
        }

        private double[] GetSkullCenter(List<string> props, string[] imageProps)
        {
            var originX = GetDoubleProp(props, imageProps, Mandibular.OriginXTag);
            var originY = GetDoubleProp(props, imageProps, Mandibular.OriginYTag);
            var originZ = GetDoubleProp(props, imageProps, Mandibular.OriginZTag);
            var spacingX = GetDoubleProp(props, imageProps, Mandibular.SpacingXTag);
            var spacingY = GetDoubleProp(props, imageProps, Mandibular.SpacingYTag);
            var spacingZ = GetDoubleProp(props, imageProps, Mandibular.SpacingZTag);

            var centerX = GetDoubleProp(props, imageProps, Mandibular.CenterXTag) * spacingX + originX;
            var centerY = GetDoubleProp(props, imageProps, Mandibular.CenterYTag) * spacingY + originY;
            var centerZ = GetDoubleProp(props, imageProps, Mandibular.CenterZTag) * spacingZ + originZ;
            return new double[] { centerX, centerY, centerZ };
        }

        private double GetDoubleProp(List<string> props, string[] imageProps, string tag)
        {
            var index = props.IndexOf(tag);
            if (index < 0) return 0d;
            return double.Parse(imageProps[index]);
        }

        public DicomSeries FirstDicom
        {
            get { return firstDicom; }
            set
            {
                if (firstDicom == value) return;
                firstDicom = value;
                OnPropertyChanged();
            }
        }
        DicomSeries firstDicom;

        public DicomSeries SecondDicom
        {
            get { return secondDicom; }
            set
            {
                if (secondDicom == value) return;
                secondDicom = value;
                OnPropertyChanged();
            }
        }
        DicomSeries secondDicom;

        public bool IsDataLoaded
        {
            get { return firstDicom.IsLoaded && secondDicom.IsLoaded; }
        }

        public PresetsViewModel Presets
        {
            get => presets ?? (presets = new PresetsViewModel());
        }
        PresetsViewModel presets;



        public bool IsTranslateOverriden
        {
            get { return isTranslateOverriden; }
            set
            {
                if (isTranslateOverriden == value) return;
                isTranslateOverriden = value;
                OnPropertyChanged();
            }
        }
        bool isTranslateOverriden;

        public ICommand OverrideTranslateCommand
        {
            get => overrideTranslateCommand ??
                (overrideTranslateCommand = new ActionCommand(ToggleOverriden));
        }
        ICommand overrideTranslateCommand;

        private void ToggleOverriden()
        {
            var loadCache = IsTranslateOverriden;
            IsTranslateOverriden = !IsTranslateOverriden;

            if (loadCache)
            {
                if (translateXCache.HasValue)
                    translateX = translateXCache.Value;

                if (translateYCache.HasValue)
                    translateY = translateYCache.Value;
                
                if (translateZCache.HasValue)
                    translateZ = translateZCache.Value;
                
                if (rotateXCache.HasValue)
                    rotateX = rotateXCache.Value;
                
                if (rotateYCache.HasValue)
                    rotateY = rotateYCache.Value;
                
                if (rotateZCache.HasValue)
                    rotateZ = rotateZCache.Value;
                
                ApplyNewRotations();
                ApplyNewTransforms();

                OnPropertyChanged(nameof(TranslateX));
                OnPropertyChanged(nameof(TranslateY));
                OnPropertyChanged(nameof(TranslateZ));
                OnPropertyChanged(nameof(RotateX));
                OnPropertyChanged(nameof(RotateY));
                OnPropertyChanged(nameof(RotateZ));
            }
        }

        public MatrixTransform3D Transform
        {
            get => transform ?? (transform = GetDefaultTransform());
        }
        MatrixTransform3D transform;

        private MatrixTransform3D GetDefaultTransform()
        {
            return new MatrixTransform3D();
        }

        public double TranslateX
        {
            get { return Math.Round(translateX, 3); }
            set
            {
                if (translateX == value) return;

                if (!translateXCache.HasValue)
                    translateXCache = translateX;

                translateX = value;
                OnPropertyChanged();

                ApplyNewTransforms();
            }
        }
        double translateX;
        double? translateXCache;

        public double TranslateY
        {
            get { return Math.Round(translateY, 3); }
            set
            {
                if (translateY == value) return;

                if (!translateYCache.HasValue)
                    translateYCache = translateY;

                translateY = value;
                OnPropertyChanged();

                ApplyNewTransforms();
            }
        }
        double translateY;
        double? translateYCache;

        public double TranslateZ
        {
            get { return Math.Round(translateZ, 3); }
            set
            {
                if (translateZ == value) return;

                if (!translateZCache.HasValue)
                    translateZCache = translateZ;

                translateZ = value;
                OnPropertyChanged();

                ApplyNewTransforms();
            }
        }
        double translateZ;
        double? translateZCache;

        public double RotateX
        {
            get { return Math.Round(rotateX, 3); }
            set
            {
                if (rotateX == value) return;
                if (!rotateXCache.HasValue)
                    rotateXCache = rotateX;
                rotateX = value;
                OnPropertyChanged();

                ApplyNewRotations();
            }
        }
        double rotateX;
        double? rotateXCache;

        public double RotateY
        {
            get { return Math.Round(rotateY, 3); }
            set
            {
                if (rotateY == value) return;
                if (!rotateYCache.HasValue)
                    rotateYCache = rotateY;
                rotateY = value;
                OnPropertyChanged();

                ApplyNewRotations();
            }
        }
        double rotateY;
        double? rotateYCache;

        public double RotateZ
        {
            get { return Math.Round(rotateZ, 3); }
            set
            {
                if (rotateZ == value) return;
                if (!rotateZCache.HasValue)
                    rotateZCache = rotateZ;

                rotateZ = value;
                OnPropertyChanged();

                ApplyNewRotations();
            }
        }
        double rotateZ;
        double? rotateZCache;

        public bool ShowSecondVolume
        {
            get { return showSecondVolume; }
            set
            {
                if (showSecondVolume == value) return;
                showSecondVolume = value;
                OnPropertyChanged();
            }
        }
        bool showSecondVolume = true;


        public BindingList<Point3D> SelectedPoints
        {
            get { return selectedPoints ?? (selectedPoints = new BindingList<Point3D>()); }
            set
            {
                if (selectedPoints == value) return;
                selectedPoints = value;
                OnPropertyChanged();
            }
        }
        BindingList<Point3D> selectedPoints;

        public BindingList<Point3D> AdditionalSelectedPoints
        {
            get { return additionalSelectedPoints ?? (additionalSelectedPoints = new BindingList<Point3D>()); }
            set
            {
                if (additionalSelectedPoints == value) return;
                additionalSelectedPoints = value;
                OnPropertyChanged();
            }
        }
        BindingList<Point3D> additionalSelectedPoints;
        double[] additionalSourcePoints;

        private void UpdateTransformElements()
        {
            var mtx = Transform.Matrix;
            translateX = mtx.M14;
            translateY = mtx.M24;
            translateZ = mtx.M34;
            OnPropertyChanged(nameof(TranslateX));
            OnPropertyChanged(nameof(TranslateY));
            OnPropertyChanged(nameof(TranslateZ));

            var rotateYRad = Math.Asin(-mtx.M13);
            rotateY = rotateYRad / Math.PI * 180;
            rotateX = Math.Acos(mtx.M33 / Math.Cos(rotateYRad)) / Math.PI * 180;
            rotateZ = Math.Acos(mtx.M11 / Math.Cos(rotateYRad)) / Math.PI * 180;
            OnPropertyChanged(nameof(RotateX));
            OnPropertyChanged(nameof(RotateY));
            OnPropertyChanged(nameof(RotateZ));
        }

        private void ApplyNewRotations()
        {
            var mtx = Transform.Matrix;
            var radX = RotateX / 180 * Math.PI;
            var radY = RotateY / 180 * Math.PI;
            var radZ = RotateZ / 180 * Math.PI;

            mtx.M11 = Math.Cos(radY) * Math.Cos(radZ);
            mtx.M12 = Math.Cos(radY) * Math.Sin(radZ);
            mtx.M13 = -Math.Sin(radY);

            mtx.M21 = Math.Sin(radX) * Math.Sin(radY) * Math.Cos(radZ) - Math.Cos(radX) * Math.Sin(radZ);
            mtx.M22 = Math.Sin(radX) * Math.Sin(radY) * Math.Sin(radZ) + Math.Cos(radX) * Math.Cos(radZ);
            mtx.M23 = Math.Sin(radX) * Math.Cos(radY);

            mtx.M31 = Math.Cos(radX) * Math.Sin(radY) * Math.Cos(radZ) + Math.Sin(radX) * Math.Sin(radZ);
            mtx.M32 = Math.Cos(radX) * Math.Sin(radY) * Math.Sin(radZ) - Math.Sin(radX) * Math.Cos(radZ);
            mtx.M33 = Math.Cos(radX) * Math.Cos(radY);

            Transform.Matrix = mtx;
            OnPropertyChanged(nameof(Transform));
            UpdateAdditionalSelectedPoints();
        }

        private void ApplyNewTransforms()
        {
            var mtx = Transform.Matrix;
            mtx.M14 = translateX;
            mtx.M24 = translateY;
            mtx.M34 = translateZ;

            Transform.Matrix = mtx;
            OnPropertyChanged(nameof(Transform));
            UpdateAdditionalSelectedPoints();
        }

        public ICommand SaveChangedResultsCommand
        {
            get => saveChangedResultsCommand ??
                (saveChangedResultsCommand = new ActionCommand(SaveChangedResults));
        }
        ICommand saveChangedResultsCommand;

        private void SaveChangedResults()
        {
            if (string.IsNullOrEmpty(resultsPath)) return;

            var csvLines = File.ReadAllLines(resultsPath).ToList();
            var props = csvLines.First().Split(';').ToList();
            var srcLine = csvLines.FirstOrDefault(x => x.Contains(firstDicom.FullPath));
            var line = csvLines.FirstOrDefault(x => x.Contains(SecondDicom.FullPath));
            var index = csvLines.IndexOf(line);

            // List:
            // - left condyle point [px]
            // - right condyle point [px]
            // - distance between condyles
            // - nasion points [px]
            // - distance from left to nasion
            // - distance from right to nasion
            // - left zygomatic
            // - right zygomatic
            // - center
            // - distance from left zyg to center
            // - distance from right zyg to center
            // todo

            // algorithm:

            var changedLine = GetChangedResultsLine(srcLine, line, props);
            csvLines[index] = changedLine;
            File.WriteAllLines(resultsPath.Replace(".csv", " - after.csv"), csvLines);
            Process.Start(Path.GetDirectoryName(resultsPath));
        }

        private string GetChangedResultsLine(string sourceLine, string targetLine, List<string> props)
        {
            var arr = targetLine.Split(';');

            var originX = GetDoubleProp(props, arr, Mandibular.OriginXTag);
            var originY = GetDoubleProp(props, arr, Mandibular.OriginYTag);
            var originZ = GetDoubleProp(props, arr, Mandibular.OriginZTag);
            var spacingX = GetDoubleProp(props, arr, Mandibular.SpacingXTag);
            var spacingY = GetDoubleProp(props, arr, Mandibular.SpacingYTag);
            var spacingZ = GetDoubleProp(props, arr, Mandibular.SpacingZTag);

            var values = arr.ToList();

            // take points in physical coord system of first image     
            var srcPoints = GetSkullPoints(props, sourceLine.Split(';'));
            var srcCenter = GetSkullCenter(props, sourceLine.Split(';'));
            var leftZyg = new double[]
            {
                srcPoints[0],
                srcPoints[1],
                srcPoints[2]
            };
            var rightZyg = new double[]
            {
                srcPoints[3],
                srcPoints[4],
                srcPoints[5]
            };
            var nasion = new double[]
            {
                srcPoints[6],
                srcPoints[7],
                srcPoints[8]
            };
            var center = new double[]
            {
                srcCenter[0],
                srcCenter[1],
                srcCenter[2]
            };

            // get stuff transformed back to second image coordinate system
            var nasionTransformed = TransformPointBack(nasion);
            var leftZygomaticTransformed = TransformPointBack(leftZyg);
            var rightZygomaticTransformed = TransformPointBack(rightZyg);
            var centerTransformed = TransformPointBack(center);

            var l2cdist = Pt2PtDistance(leftZygomaticTransformed, centerTransformed);
            var r2cdist = Pt2PtDistance(rightZygomaticTransformed, centerTransformed);

            // get stuff in pixels in order to overwrite values
            var nasionPx = new double[]
            {
                (nasionTransformed[0] - originX) / spacingX,
                (nasionTransformed[1] - originY) / spacingY,
                (nasionTransformed[2] - originZ) / spacingZ,
            };
            var leftZygPx = new double[]
{
                (leftZygomaticTransformed[0] - originX) / spacingX,
                (leftZygomaticTransformed[1] - originY) / spacingY,
                (leftZygomaticTransformed[2] - originZ) / spacingZ,
            };
            var rightZygPx = new double[]
            {
                (rightZygomaticTransformed[0] - originX) / spacingX,
                (rightZygomaticTransformed[1] - originY) / spacingY,
                (rightZygomaticTransformed[2] - originZ) / spacingZ,
            };
            var centerPx = new double[]
            {
                (centerTransformed[0] - originX) / spacingX,
                (centerTransformed[1] - originY) / spacingY,
                (centerTransformed[2] - originZ) / spacingZ,
            };

            // calculate new coordinate system again
            Vector3D zygVec = new Vector3D
            {
                X = rightZygomaticTransformed[0] - leftZygomaticTransformed[0],
                Y = rightZygomaticTransformed[1] - leftZygomaticTransformed[1],
                Z = rightZygomaticTransformed[2] - leftZygomaticTransformed[2]
            };

            Vector3D zyg2NVec = new Vector3D
            {
                X = nasionTransformed[0] - leftZygomaticTransformed[0],
                Y = nasionTransformed[1] - leftZygomaticTransformed[1],
                Z = nasionTransformed[2] - leftZygomaticTransformed[2]
            };

            var k = Vector3D.DotProduct(zyg2NVec, zygVec) / Vector3D.DotProduct(zygVec, zygVec);

            Vector3D newOrigin = new Vector3D
            {
                X = leftZygomaticTransformed[0] + k * zygVec.X,
                Y = leftZygomaticTransformed[1] + k * zygVec.Y,
                Z = leftZygomaticTransformed[2] + k * zygVec.Z
            };

            Vector3D xAxis = new Vector3D
            {
                X = leftZygomaticTransformed[0] - newOrigin.X,
                Y = leftZygomaticTransformed[1] - newOrigin.Y,
                Z = leftZygomaticTransformed[2] - newOrigin.Z
            };
            xAxis.Normalize();
            Vector3D zAxis = new Vector3D
            {
                X = nasionTransformed[0] - newOrigin.X,
                Y = nasionTransformed[1] - newOrigin.Y,
                Z = nasionTransformed[2] - newOrigin.Z
            };
            zAxis.Normalize();
            Vector3D yAxis = Vector3D.CrossProduct(zAxis, xAxis);
            yAxis.Normalize();

            var newOriginPoints = new double[9]
            {
                newOrigin.X + xAxis.X, newOrigin.Y + xAxis.Y, newOrigin.Z + xAxis.Z,
                newOrigin.X + yAxis.X, newOrigin.Y + yAxis.Y, newOrigin.Z + yAxis.Z,
                newOrigin.X + zAxis.X, newOrigin.Y + zAxis.Y, newOrigin.Z + zAxis.Z
            };
            var newOriginCenter = new double[3] { newOrigin.X, newOrigin.Y, newOrigin.Z };
            var oldOriginPoints = new double[9] { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
            var oldOriginCenter = new double[3] { 0, 0, 0 };

            var trPtr = DicomProcessing_GetTransform(newOriginPoints, newOriginCenter, oldOriginPoints, oldOriginCenter);
            var trArr = new double[16];
            Marshal.Copy(trPtr, trArr, 0, 16);

            // calcultae condyles coords again, with new skulls points
            var leftCondyle = new double[]
            {
                double.Parse(values[props.IndexOf(Mandibular.LeftCondyleXTag)]) * spacingX + originX,
                double.Parse(values[props.IndexOf(Mandibular.LeftCondyleYTag)]) * spacingY + originY,
                double.Parse(values[props.IndexOf(Mandibular.LeftCondyleZTag)]) * spacingZ + originZ
            };
            var rightCondyle = new double[]
            {
                double.Parse(values[props.IndexOf(Mandibular.RightCondyleXTag)]) * spacingX + originX,
                double.Parse(values[props.IndexOf(Mandibular.RightCondyleYTag)]) * spacingY + originY,
                double.Parse(values[props.IndexOf(Mandibular.RightCondyleZTag)]) * spacingZ + originZ
            };

            var leftCondyleReal = new double[3]
            {
                leftCondyle[0] * trArr[0] + leftCondyle[1] * trArr[1] + leftCondyle[2] * trArr[2] + trArr[3],
                leftCondyle[0] * trArr[4] + leftCondyle[1] * trArr[5] + leftCondyle[2] * trArr[6] + trArr[7],
                leftCondyle[0] * trArr[8] + leftCondyle[1] * trArr[9] + leftCondyle[2] * trArr[10] + trArr[11]
            };
            var rightCondyleReal = new double[3]
            {
                rightCondyle[0] * trArr[0] + rightCondyle[1] * trArr[1] + rightCondyle[2] * trArr[2] + trArr[3],
                rightCondyle[0] * trArr[4] + rightCondyle[1] * trArr[5] + rightCondyle[2] * trArr[6] + trArr[7],
                rightCondyle[0] * trArr[8] + rightCondyle[1] * trArr[9] + rightCondyle[2] * trArr[10] + trArr[11]
            };


            var l2ndist = Pt2PtDistance(leftCondyle, nasionTransformed);
            var r2ndist = Pt2PtDistance(rightCondyle, nasionTransformed);

            values[props.IndexOf(Mandibular.NasionXTag)] = ((int)((nasionTransformed[0] - originX) / spacingX)).ToString();
            values[props.IndexOf(Mandibular.NasionYTag)] = ((int)((nasionTransformed[1] - originY) / spacingY)).ToString();
            values[props.IndexOf(Mandibular.NasionZTag)] = ((int)((nasionTransformed[2] - originZ) / spacingZ)).ToString();

            values[props.IndexOf(Mandibular.LeftZygomaticXTag)] = ((int)((leftZygomaticTransformed[0] - originX) / spacingX)).ToString();
            values[props.IndexOf(Mandibular.LeftZygomaticYTag)] = ((int)((leftZygomaticTransformed[1] - originY) / spacingY)).ToString();
            values[props.IndexOf(Mandibular.LeftZygomaticZTag)] = ((int)((leftZygomaticTransformed[2] - originZ) / spacingZ)).ToString();

            values[props.IndexOf(Mandibular.RightZygomaticXTag)] = ((int)((rightZygomaticTransformed[0] - originX) / spacingX)).ToString();
            values[props.IndexOf(Mandibular.RightZygomaticYTag)] = ((int)((rightZygomaticTransformed[1] - originY) / spacingY)).ToString();
            values[props.IndexOf(Mandibular.RightZygomaticZTag)] = ((int)((rightZygomaticTransformed[2] - originZ) / spacingZ)).ToString();

            values[props.IndexOf(Mandibular.CenterXTag)] = ((int)((centerTransformed[0] - originX) / spacingX)).ToString();
            values[props.IndexOf(Mandibular.CenterYTag)] = ((int)((centerTransformed[1] - originY) / spacingY)).ToString();
            values[props.IndexOf(Mandibular.CenterZTag)] = ((int)((centerTransformed[2] - originZ) / spacingZ)).ToString();

            values[props.IndexOf(Mandibular.LeftConToNasionDistanceTag)] = Math.Round(l2ndist, 3).ToString();
            values[props.IndexOf(Mandibular.RightConToNasionDistanceTag)] = Math.Round(r2ndist, 3).ToString();
            values[props.IndexOf(Mandibular.LeftZygToCenterDistanceTag)] = Math.Round(l2cdist, 3).ToString();
            values[props.IndexOf(Mandibular.RightZygToCenterDistanceTag)] = Math.Round(r2cdist, 3).ToString();

            values[props.IndexOf(Mandibular.LeftCondyleXRealTag)] = Math.Round(leftCondyleReal[0], 3).ToString();
            values[props.IndexOf(Mandibular.LeftCondyleYRealTag)] = Math.Round(leftCondyleReal[1], 3).ToString();
            values[props.IndexOf(Mandibular.LeftCondyleZRealTag)] = Math.Round(leftCondyleReal[2], 3).ToString();
            values[props.IndexOf(Mandibular.RightCondyleXRealTag)] = Math.Round(rightCondyleReal[0], 3).ToString();
            values[props.IndexOf(Mandibular.RightCondyleYRealTag)] = Math.Round(rightCondyleReal[1], 3).ToString();
            values[props.IndexOf(Mandibular.RightCondyleZRealTag)] = Math.Round(rightCondyleReal[2], 3).ToString();

            string newLine = string.Empty;
            for (int i = 0; i < values.Count; i++)
            {
                newLine += values[i];
                if (i != values.Count - 1)
                    newLine += ";";
            }

            return newLine;
        }

        private double[] TransformPoint(double[] pt)
        {
            var mtx = transform.Matrix;
            return new double[]
            {
                pt[0] * mtx.M11 + pt[1] * mtx.M12 + pt[2] * mtx.M13 + mtx.M14,
                pt[0] * mtx.M21 + pt[1] * mtx.M22 + pt[2] * mtx.M23 + mtx.M24,
                pt[0] * mtx.M31 + pt[1] * mtx.M32 + pt[2] * mtx.M33 + mtx.M34,
            };
        }

        private double[] TransformPointBack(double[] pt)
        {
            var mtx = transform.Matrix;
            return new double[]
            {
                pt[0] * mtx.M11 + pt[1] * mtx.M21 + pt[2] * mtx.M31 - mtx.M14,
                pt[0] * mtx.M12 + pt[1] * mtx.M22 + pt[2] * mtx.M32 - mtx.M24,
                pt[0] * mtx.M13 + pt[1] * mtx.M23 + pt[2] * mtx.M33 - mtx.M34,
            };
        }

        private double Pt2PtDistance(double[] p1, double[] p2)
        {
            return Math.Sqrt(
                Math.Pow(p1[0] - p2[0], 2)
                + Math.Pow(p1[1] - p2[1], 2)
                + Math.Pow(p1[2] - p2[2], 2)
                );
        }
    }
}
