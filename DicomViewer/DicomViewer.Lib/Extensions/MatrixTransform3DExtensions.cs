using System.Windows.Media.Media3D;

namespace DicomViewer.Lib.Extensions
{
    public static class MatrixTransform3DExtensions
    {
        public static double[] ToArray(this MatrixTransform3D transform)
        {
            var mtx = transform.Matrix;
            var arr = new double[16];
            arr[0] = mtx.M11;
            arr[1] = mtx.M12;
            arr[2] = mtx.M13;
            arr[3] = mtx.M14;

            arr[4] = mtx.M21;
            arr[5] = mtx.M22;
            arr[6] = mtx.M23;
            arr[7] = mtx.M24;

            arr[8] = mtx.M31;
            arr[9] = mtx.M32;
            arr[10] = mtx.M33;
            arr[11] = mtx.M34;

            arr[12] = 0;
            arr[13] = 0;
            arr[14] = 0;
            arr[15] = mtx.M44;

            return arr;
        }
    }
}
