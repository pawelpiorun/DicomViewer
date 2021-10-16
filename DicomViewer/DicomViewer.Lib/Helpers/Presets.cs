using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DicomViewer.Lib.Helpers
{
    public sealed class Presets
    {
        #region Extern

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern IntPtr Presets_GetInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern IntPtr Presets_CreatePreset(IntPtr instance);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_SetName(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_SetValues(IntPtr instance, double specularPower, double specular, double shade, double ambient, double diffuse);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_AddGradientPoint(IntPtr instance, double x, double y);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_AddScalarPoint(IntPtr instance, double x, double y);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_AddColorTransfer(IntPtr instance, double x, double r, double g, double b);

        #endregion

        #region Singleton
        private Presets()
        {
            unmanagedPresets = Presets_GetInstance();
        }

        public static Presets Instance
        {
            get
            {
                return instance ?? (instance = new Presets());
            }
        }
        static Presets instance;

        public IntPtr GetPresetsInstance()
        {
            return unmanagedPresets;
        }
        static IntPtr unmanagedPresets;
        #endregion

        // TODO: load from .xml files, share with unmanaged code
        public List<Preset> GetDefaultPresets()
        {
            return presets ?? (presets = LoadPresets());
        }
        private List<Preset> presets;

        private List<Preset> LoadPresets()
        {
            var managedPresets = new List<Preset>()
            {
                PresetFactory.Create(PresetType.CTBone),
                PresetFactory.Create(PresetType.CTAAA),
                PresetFactory.Create(PresetType.CTAAA2),
                PresetFactory.Create(PresetType.CTBones),
                PresetFactory.Create(PresetType.CTFat),
                PresetFactory.Create(PresetType.CTLung),
                PresetFactory.Create(PresetType.CTMIP),
                PresetFactory.Create(PresetType.CTMuscle),
                PresetFactory.Create(PresetType.CTSoftTissue),
                PresetFactory.Create(PresetType.General),
                PresetFactory.Create(PresetType.AirVolume),
                PresetFactory.Create(PresetType.Mandibular)

            };

            for (int i = 0; i < managedPresets.Count; i++)
            {
                var preset = managedPresets[i];
                var pointer = Presets_CreatePreset(unmanagedPresets);
                Preset_SetName(pointer, preset.Name);
                Preset_SetValues(pointer, preset.SpecularPower, preset.Specular,
                    preset.Shade, preset.Ambient, preset.Diffuse);

                foreach (var gpoint in preset.GradientOpacity)
                    Preset_AddGradientPoint(pointer, gpoint.x, gpoint.y);

                foreach (var spoint in preset.ScalarOpacity)
                    Preset_AddScalarPoint(pointer, spoint.x, spoint.y);

                foreach (var cpoint in preset.ColorTransfer)
                    Preset_AddColorTransfer(pointer, cpoint.x, cpoint.r, cpoint.g, cpoint.b);

                preset.UnmanagedInstance = pointer;
            }

            return managedPresets;
        }
    }

    public class Preset
    {
        public string Name { get; set; }

        // temporary
        public PresetType PresetType { get; set; }
        public IntPtr UnmanagedInstance { get; set; }

        public PointXY[] GradientOpacity { get; set; }
        public double SpecularPower { get; set; }
        public PointXY[] ScalarOpacity { get; set; }
        public double Specular { get; set; }
        public double Shade { get; set; }
        public double Ambient { get; set; }
        public RGBColorTransfer[] ColorTransfer { get; set; }
        public double Diffuse { get; set; }
    }

    public struct PointXY
    {
        public PointXY(double x, double y)
        {
            this.x = x;
            this.y = y;
        }
        public double x;
        public double y;
    }

    public struct RGBColorTransfer
    {
        public RGBColorTransfer(double x, double r,
            double g, double b)
        {
            this.x = x;
            this.r = r;
            this.g = g;
            this.b = b;
        }
        public double x;
        public double r;
        public double g;
        public double b;
    }

    // TODO: Preset factory with structures shared with unmanaged code

    internal static class PresetFactory
    {
        #region Extern

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern IntPtr Presets_GetInstance();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern IntPtr Presets_CreatePreset();

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_SetName(IntPtr instance, [MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_SetValues(IntPtr instance, double specularPower, double specular, double shade, double ambient, double diffuse);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_AddGradientPoint(IntPtr instance, double x, double y);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_AddScalarPoint(IntPtr instance, double x, double y);

        [DllImport("BimaxProcessing.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        static extern void Preset_AddColorTransfer(IntPtr instance, double x, double r, double g, double b);

        #endregion

        internal static Preset Create(PresetType preset)
        {
            switch (preset)
            {
                case PresetType.CTAAA:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTAAA,
                            Name = "CT-AAA",
                            SpecularPower = 10,
                            Specular = 0.2,
                            Shade = 1,
                            Ambient = 0.1,
                            Diffuse = 0.9,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(255,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -3024,    y: 0),
                                new PointXY(x: 143.556,  y: 0),
                                new PointXY(x: 166.222,  y: 0.686275),
                                new PointXY(x: 214.389,  y: 0.696078),
                                new PointXY(x: 419.736,  y: 0.833333),
                                new PointXY(x: 3071,     y: 0.803922),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -3024,   r: 0,        g: 0,        b:   0),
                                new RGBColorTransfer(x: 143.556, r: 0.615686, g: 0.356863, b: 0.184314),
                                new RGBColorTransfer(x: 166.222, r: 0.882353, g: 0.603922, b: 0.290196),
                                new RGBColorTransfer(x: 214.389, r: 1,        g: 1,        b: 1),
                                new RGBColorTransfer(x: 419.736, r: 1,        g: 0.937033, b: 0.954531),
                                new RGBColorTransfer(x: 3071,    r: 0.827451, g: 0.658824, b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTAAA2:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTAAA2,
                            Name = "CT-AAA2",
                            SpecularPower = 10,
                            Specular = 0.2,
                            Shade = 1,
                            Ambient = 0.1,
                            Diffuse = 0.9,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(255,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -3024,   y: 0),
                                new PointXY(x: 129.542, y: 0),
                                new PointXY(x: 145.244, y: 0.166667),
                                new PointXY(x: 157.02,  y: 0.5),
                                new PointXY(x: 169.918, y: 0.627451),
                                new PointXY(x: 395.575, y: 0.8125),
                                new PointXY(x: 1578.73, y: 0.8125),
                                new PointXY(x: 3071,    y: 0.8125),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -3024,   r: 0,       g: 0,        b:   0),
                                new RGBColorTransfer(x: 129.542, r: 0.54902, g: 0.25098,  b: 0.14902),
                                new RGBColorTransfer(x: 145.244, r: 0.6,     g: 0.627451, b: 0.843137),
                                new RGBColorTransfer(x: 157.02, r: 0.890196, g: 0.47451,  b: 0.6),
                                new RGBColorTransfer(x: 169.918, r: 0.992157, g: 0.870588,  b: 0.392157),
                                new RGBColorTransfer(x: 395.575, r: 1,        g: 0.886275, b: 0.658824),
                                new RGBColorTransfer(x: 1578.73, r: 1,        g: 0.829256, b: 0.957922),
                                new RGBColorTransfer(x: 3071,    r: 0.827451, g: 0.658824, b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTBone:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTBone,
                            Name = "CT-Bone",
                            SpecularPower = 10,
                            Specular = 0.2,
                            Shade = 1,
                            Ambient = 0.1,
                            Diffuse = 0.9,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(255,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -3024,    y: 0),
                                new PointXY(x: -16.4458, y: 0),
                                new PointXY(x: 641.385,  y: 0.715686),
                                new PointXY(x: 3071,     y: 0.705882),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -3024,   r: 0,        g: 0,        b:   0),
                                new RGBColorTransfer(x: 16.4458, r: 0.729412, g: 0.254902, b: 0.301961),
                                new RGBColorTransfer(x: 641.385, r: 0.905882, g: 0.815686, b: 0.552941),
                                new RGBColorTransfer(x: 3071,    r: 1,        g: 1,        b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTBones:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTBones,
                            Name = "CT-Bones",
                            SpecularPower = 1,
                            Specular = 0,
                            Shade = 1,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(985.12,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -1000,  y: 0),
                                new PointXY(x: 152.19, y: 0),
                                new PointXY(x: 278.93, y: 0.190476),
                                new PointXY(x: 952,    y: 0.2),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -1000,  r: 0.3, g: 0.3,      b:   1),
                                new RGBColorTransfer(x: -488,   r: 0.3, g: 1,        b: 0.3),
                                new RGBColorTransfer(x: 463.28, r: 1,   g: 0,        b: 0),
                                new RGBColorTransfer(x: 659.15, r: 1,   g: 0.912535, b: 0.0374849),
                                new RGBColorTransfer(x: 953,    r: 1,   g: 0.3,      b: 0.3),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTFat:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTFat,
                            Name = "CT-Fat",
                            SpecularPower = 1,
                            Specular = 0,
                            Shade = 1,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,      1),
                                new PointXY(985.12, 1),
                                new PointXY(988,    1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -1000, y: 0),
                                new PointXY(x: -100,  y: 0),
                                new PointXY(x: -99,   y: 0.15),
                                new PointXY(x: -60,   y: 0.15),
                                new PointXY(x: -59,   y: 0),
                                new PointXY(x: 101.2, y: 0),
                                new PointXY(x: 952,   y: 0),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -1000,   r: 0.3,      g: 0.3,      b:   1),
                                new RGBColorTransfer(x: -497.5,  r: 0.3,      g: 1,        b: 0.3),
                                new RGBColorTransfer(x: -99,     r: 0,        g: 0,        b: 1),
                                new RGBColorTransfer(x: -76.946, r: 0,        g: 1,        b: 0),
                                new RGBColorTransfer(x: -65.481, r: 0.835431, g: 0.888889, b: 0.0165387),
                                new RGBColorTransfer(x: 83.89,   r: 1,        g: 0,        b: 0),
                                new RGBColorTransfer(x: 463.28,  r: 1,        g: 0,        b: 0),
                                new RGBColorTransfer(x: 659.15,  r: 1,        g: 0.912535, b: 0.0374849),
                                new RGBColorTransfer(x: 2952,    r: 1,        g: 0.300267, b: 0.299886),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTLung:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTLung,
                            Name = "CT-Lung",
                            SpecularPower = 1,
                            Specular = 0,
                            Shade = 1,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,      1),
                                new PointXY(985.12, 1),
                                new PointXY(988,    1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -1000, y: 0),
                                new PointXY(x: -600,  y: 0),
                                new PointXY(x: -599,  y: 0.15),
                                new PointXY(x: -400,  y: 0.15),
                                new PointXY(x: -399,  y: 0),
                                new PointXY(x: 2952,  y: 0),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -1000, r: 0.3,      g: 0.3,      b:   1),
                                new RGBColorTransfer(x: -600,  r: 0,        g: 0,        b: 1),
                                new RGBColorTransfer(x: -530,  r: 0.134704, g: 0.781726, b: 0.0724558),
                                new RGBColorTransfer(x: -460,  r: 0.929244, g: 1,        b: 0.109473),
                                new RGBColorTransfer(x: -400,  r: 0.888889, g: 0.254949, b: 0.0240258),
                                new RGBColorTransfer(x: 2952,  r: 1,        g: 0.3,      b: 0.3),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTMIP:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTMIP,
                            Name = "CT-MIP",
                            SpecularPower = 10,
                            Specular = 0.2,
                            Shade = 1,
                            Ambient = 0.1,
                            Diffuse = 0.9,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,   1),
                                new PointXY(255, 1),
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -3024,   y: 0),
                                new PointXY(x: -637.62, y: 0),
                                new PointXY(x: 700,     y: 1),
                                new PointXY(x: 3071,    y: 1),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -3024,   r: 0, g: 0, b: 0),
                                new RGBColorTransfer(x: -637.62, r: 1, g: 1, b: 1),
                                new RGBColorTransfer(x: 700,     r: 1, g: 1, b: 1),
                                new RGBColorTransfer(x: 3071,    r: 1, g: 1, b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTMuscle:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTMuscle,
                            Name = "CT-Muscle",
                            SpecularPower = 10,
                            Specular = 0.2,
                            Shade = 1,
                            Ambient = 0.1,
                            Diffuse = 0.9,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,   1),
                                new PointXY(255, 1),
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -3024,    y: 0),
                                new PointXY(x: -155.407, y: 0),
                                new PointXY(x: 217.641,  y: 0.676471),
                                new PointXY(x: 3071,     y: 0.803922),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -3024,    r: 0,        g: 0,        b: 0),
                                new RGBColorTransfer(x: -155.407, r: 0.54902,  g: 0.25098,  b: 0.14902),
                                new RGBColorTransfer(x: 217.641,  r: 0.882353, g: 0.603922, b: 0.290196),
                                new RGBColorTransfer(x: 419.736,  r: 1,        g: 0.937033, b: 0.954531),
                                new RGBColorTransfer(x: 3071,     r: 0.827451, g: 0.658824, b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.CTSoftTissue:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.CTSoftTissue,
                            Name = "CT-Soft-Tissue",
                            SpecularPower = 10,
                            Specular = 0,
                            Shade = 1,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,   1),
                                new PointXY(255, 1),
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -2048,   y: 0),
                                new PointXY(x: -167.01, y: 0),
                                new PointXY(x: -160,    y: 1),
                                new PointXY(x: 240,     y: 1),
                                new PointXY(x: 3661,    y: 1),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -2048,   r: 0,         g: 0,         b: 0),
                                new RGBColorTransfer(x: -167.01, r: 0,         g: 0,         b: 0),
                                new RGBColorTransfer(x: -160,    r: 0.0556356, g: 0.0556356, b: 0.0556356),
                                new RGBColorTransfer(x: 240,     r: 1,         g: 1,         b: 1),
                                new RGBColorTransfer(x: 3661,    r: 1,         g: 1,         b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,   g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.MRDefault:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.MRDefault,
                            Name = "MR-Default",
                            SpecularPower = 1,
                            Specular = 0,
                            Shade = 1,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(255,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: 0,    y: 0),
                                new PointXY(x: 20,   y: 0),
                                new PointXY(x: 40,   y: 0.15),
                                new PointXY(x: 120,  y: 0.3),
                                new PointXY(x: 220,  y: 0.375),
                                new PointXY(x: 1024, y: 0.5),
                                new PointXY(x: short.MaxValue,y: 0.7) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: 0,    r: 0,        g: 0,        b: 0),
                                new RGBColorTransfer(x: 20,   r: 0.168627, g: 0,        b: 0),
                                new RGBColorTransfer(x: 40,   r: 0.403922, g: 0.145098, b: 0.0784314),
                                new RGBColorTransfer(x: 120,  r: 0.780392, g: 0.607843, b: 0.380392),
                                new RGBColorTransfer(x: 220,  r: 0.847059, g: 0.835294, b: 0.788235),
                                new RGBColorTransfer(x: 1024, r: 1,        g: 1,        b: 1),
                                new RGBColorTransfer(x: short.MaxValue,r: 1,        g: 0,        b: 0) // selection
                            }
                        };
                    }
                case PresetType.MRAngio:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.MRAngio,
                            Name = "MR-Angio",
                            SpecularPower = 1,
                            Specular = 0,
                            Shade = 1,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(255,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -2048,   y: 0),
                                new PointXY(x: 151.354, y: 0),
                                new PointXY(x: 158.279, y: 0.4375),
                                new PointXY(x: 190.112, y: 0.580357),
                                new PointXY(x: 200.873, y: 0.732143),
                                new PointXY(x: 3661,    y: 0.741071),
                                new PointXY(x: short.MaxValue, y: 0.8) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -2048,    r: 0,        g: 0,        b: 0),
                                new RGBColorTransfer(x: 151.354,  r: 0,        g: 0,        b: 0),
                                new RGBColorTransfer(x: 158.279,  r: 0.74902,  g: 0.376471, b: 0),
                                new RGBColorTransfer(x: 190.112,  r: 1,        g: 0.86667,  b: 0.733333),
                                new RGBColorTransfer(x: 200.873,  r: 0.937255, g: 0.937255, b: 0.937255),
                                new RGBColorTransfer(x: 3661,     r: 1,        g: 1,        b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,   g: 0,        b: 0) // selection
                            }
                        };
                    }
                case PresetType.MRMIP:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.MRMIP,
                            Name = "MR-MIP",
                            SpecularPower = 1,
                            Specular = 0,
                            Shade = 0,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(255,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: 0,       y: 0),
                                new PointXY(x: 98.3725, y: 0),
                                new PointXY(x: 416.637, y: 1),
                                new PointXY(x: 2800,    y: 1),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: 0,        r: 1, g: 1, b: 1),
                                new RGBColorTransfer(x: 98.3725,  r: 1, g: 1, b: 1),
                                new RGBColorTransfer(x: 416.637,  r: 1, g: 1, b: 1),
                                new RGBColorTransfer(x: 2800,     r: 1, g: 1, b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1, g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.MRT2Brain:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.MRT2Brain,
                            Name = "MR-T2-BRAIN",
                            SpecularPower = 40,
                            Specular = 0.5,
                            Shade = 1,
                            Ambient = 0.3,
                            Diffuse = 0.6,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,1),
                                new PointXY(160.25,1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x:0,       y: 0),
                                new PointXY(x:36.05,   y: 0),
                                new PointXY(x:218.302, y: 0.171429),
                                new PointXY(x:412.406, y: 1),
                                new PointXY(x:641,     y: 1),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: 0,        r: 0,        g: 0,        b: 0),
                                new RGBColorTransfer(x: 98.7223,  r: 0.956863, g: 0.839216, b: 0.192157),
                                new RGBColorTransfer(x: 412.406,  r: 0,        g: 0.592157, b: 0.807843),
                                new RGBColorTransfer(x: 641,      r: 1,        g: 1,        b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,   g: 0,        b: 0) // selection
                            },
                        };
                    }
                case PresetType.General:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.General,
                            Name = "General",
                            SpecularPower = 10,
                            Specular = 0.2,
                            Shade = 1,
                            Ambient = 0.1,
                            Diffuse = 0.9,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    0),
                                new PointXY(2000,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -800,  y: 0),
                                new PointXY(x: -750,  y: 1),
                                new PointXY(x: -350,  y: 1),
                                new PointXY(x: -300,  y: 0),
                                new PointXY(x: -200,  y: 0),
                                new PointXY(x: -100,  y: 1),
                                new PointXY(x: 1000,  y: 0),
                                new PointXY(x: 2750,  y: 0),
                                new PointXY(x: 2976,  y: 1),
                                new PointXY(x: 3000,  y: 0),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -750,  r: 0.08, g: 0.05, b: 0.03),
                                new RGBColorTransfer(x: -350,  r: 0.39, g: 0.25, b: 0.16),
                                new RGBColorTransfer(x: -200,  r: 0.80, g: 0.80, b: 0.80),
                                new RGBColorTransfer(x: 2750,  r: 0.70, g: 0.70, b: 0.70),
                                new RGBColorTransfer(x: 3000,  r: 0.35, g: 0.35, b: 0.35),
                                new RGBColorTransfer(x: short.MaxValue, r:1, g: 0, b: 0) // selection
                            }
                        };
                    }
                case PresetType.AirVolume:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.AirVolume,
                            Name = "AirVolume",
                            SpecularPower = 1,
                            Specular = 0,
                            Shade = 1,
                            Ambient = 0.2,
                            Diffuse = 1,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,      1),
                                new PointXY(255,    1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -1000, y: 0),
                                new PointXY(x: -600,  y: 0),
                                new PointXY(x: -599,  y: 0.15),
                                new PointXY(x: -400,  y: 0.15),
                                new PointXY(x: -399,  y: 0),
                                new PointXY(x: 2000,  y: 0),
                                new PointXY(x: 3000,  y: 0.1), // selection
                                new PointXY(x: short.MaxValue, y: 1) // minimal path
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -1000, r: 0.8, g: 0.8, b: 0.8),
                                new RGBColorTransfer(x: -400,  r: 0.8, g: 0.8, b: 0.8),
                                new RGBColorTransfer(x: 2990,  r: 1,   g: 1,   b: 1),
                                new RGBColorTransfer(x: 3000,  r: 1,   g: 0,   b: 0), // selection
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 1, b: 1) 
                            }
                        };
                    }
                case PresetType.Mandibular:
                    {
                        return new Preset
                        {
                            PresetType = PresetType.Mandibular,
                            Name = "Mandibular",
                            SpecularPower = 10,
                            Specular = 0.2,
                            Shade = 1,
                            Ambient = 0.1,
                            Diffuse = 0.9,
                            GradientOpacity = new PointXY[]
                            {
                                new PointXY(0,    1),
                                new PointXY(255,  1)
                            },
                            ScalarOpacity = new PointXY[]
                            {
                                new PointXY(x: -3024,   y: 0),
                                new PointXY(x: 129.542, y: 0),
                                new PointXY(x: 145.244, y: 0.166667),
                                new PointXY(x: 157.02,  y: 0.5),
                                new PointXY(x: 169.918, y: 0.627451),
                                new PointXY(x: 395.575, y: 0.8125),
                                new PointXY(x: 1578.73, y: 0.8125),
                                new PointXY(x: 3071,    y: 0.8125),
                                new PointXY(x: 5000,    y: 1),
                                new PointXY(x: short.MaxValue, y: 1) // selection
                            },
                            ColorTransfer = new RGBColorTransfer[]
                            {
                                new RGBColorTransfer(x: -3024,   r: 0,       g: 0,        b:   0),
                                new RGBColorTransfer(x: 129.542, r: 0.54902, g: 0.25098,  b: 0.14902),
                                new RGBColorTransfer(x: 145.244, r: 0.6,     g: 0.627451, b: 0.843137),
                                new RGBColorTransfer(x: 157.02, r: 0.890196, g: 0.47451,  b: 0.6),
                                new RGBColorTransfer(x: 169.918, r: 0.992157, g: 0.870588,  b: 0.392157),
                                new RGBColorTransfer(x: 395.575, r: 1,        g: 0.886275, b: 0.658824),
                                new RGBColorTransfer(x: 1578.73, r: 1,        g: 0.829256, b: 0.957922),
                                new RGBColorTransfer(x: 3071,    r: 0.827451, g: 0.658824, b: 1),
                                new RGBColorTransfer(x: 5000,    r: 0.2,        g: 1, b: 0.2),
                                new RGBColorTransfer(x: 6000,    r: 1,        g: 1, b: 1),
                                new RGBColorTransfer(x: short.MaxValue, r:1,  g: 0.829256, b: 0.957922) // selection
                            }
                        };
                    }
                default:
                    throw new NotImplementedException();

            }
        }
    }


    public enum PresetType
    {
        CTAAA,
        CTAAA2,
        CTBone,
        CTBones,
        CTFat,
        CTLung,
        CTMIP,
        CTMuscle,
        CTSoftTissue,
        MRDefault,
        MRAngio,
        MRMIP,
        MRT2Brain,
        General,

        AirVolume,
        Mandibular
    }
}
