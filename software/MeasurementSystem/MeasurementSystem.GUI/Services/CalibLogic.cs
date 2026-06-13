using System;
using System.Collections.Generic;
using System.Linq;

namespace MeasurementSystem.GUI.Services
{
    public class CalibLogic
    {
        // Danh sách lưu trữ các cặp điểm: X (Raw ADC) và Y (Giá trị thật)
        private List<double> _rawPoints = new List<double>();
        private List<double> _realPoints = new List<double>();

        public void SavePoint(double raw, double real)
        {
            _rawPoints.Add(raw);
            _realPoints.Add(real);
        }

        public void ClearPoints()
        {
            _rawPoints.Clear();
            _realPoints.Clear();
        }

        // Hàm tính toán hệ số a, b dựa trên phương pháp hồi quy tuyến tính
        public (float a, float b) Calculate()
        {
            if (_rawPoints.Count < 2)
                throw new Exception("Cần ít nhất 2 điểm để tính toán!");

            // Tính toán dựa trên 2 điểm cuối cùng (Linear Interpolation)
            double x1 = _rawPoints[_rawPoints.Count - 2];
            double y1 = _realPoints[_realPoints.Count - 2];
            double x2 = _rawPoints[_rawPoints.Count - 1];
            double y2 = _realPoints[_realPoints.Count - 1];

            // Công thức: a = (y2 - y1) / (x2 - x1)
            float a = (float)((y2 - y1) / (x2 - x1));
            // Công thức: b = y1 - a * x1
            float b = (float)(y1 - a * x1);

            return (a, b);
        }

        public int PointCount => _rawPoints.Count;
    }
}