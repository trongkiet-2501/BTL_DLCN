using System.Collections.Generic;
using MeasurementSystem.Backend.Models;

namespace MeasurementSystem.Backend.Services
{
    public class CalibrationService
    {
        private Dictionary<string, Calibration> _calibMap = new();

        public void SetCalibration(string sensor, float a, float b)
        {
            _calibMap[sensor] = new Calibration { A = a, B = b };
        }

        public float Apply(string sensor, float raw)
        {
            if (_calibMap.ContainsKey(sensor))
            {
                return _calibMap[sensor].Apply(raw);
            }
            return raw; // nếu chưa calib
        }
    }
}