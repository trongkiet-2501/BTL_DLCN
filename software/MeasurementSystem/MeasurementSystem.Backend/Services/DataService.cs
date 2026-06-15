using MeasurementSystem.Backend.Models;
using System;
using System.Globalization;

namespace MeasurementSystem.Backend.Services
{
    public class DataService
    {
        private SerialService _serial;
        private CalibrationService _calibService;
        private CommandService _commandService;

        public event Action<SensorData>? OnDataUpdated;
        public event Action<string>? OnError;

        public DataService(SerialService serial)
        {
            _serial = serial;

            _calibService = new CalibrationService();
            _commandService = new CommandService(serial);

            _serial.OnRawDataReceived += HandleRawData;

            _serial.OnDisconnected += () =>
            {
                OnError?.Invoke("Serial disconnected");
            };
        }

        private void HandleRawData(string raw)
        {
            try
            {
                var data = ParseData(raw);
                if (data != null)
                {
                    // Apply calibration
                    data.Temperature = _calibService.Apply("THERMISTOR", data.Temperature);
                    data.LaserDistance = _calibService.Apply("LASER", data.LaserDistance);
                    data.Angle = _calibService.Apply("POTENTIOMETER", data.Angle);
                    data.UltrasonicDistance = _calibService.Apply("ULTRASONIC", data.UltrasonicDistance);

                    OnDataUpdated?.Invoke(data);
                }
            }
            catch (Exception ex)
            {
                OnError?.Invoke("Parse error: " + ex.Message);
            }
        }

        private SensorData ParseData(string raw)
        {
            if (string.IsNullOrWhiteSpace(raw))
                throw new Exception("Empty data");

            Console.WriteLine("RAW: " + raw);

            var parts = raw.Split(';');

            SensorData data = new SensorData();

            bool hasTherm = false, hasLaser = false, hasPot = false, hasUltra = false;

            foreach (var part in parts)
            {
                var kv = part.Split(':');

                if (kv.Length != 2)
                    throw new Exception($"Invalid format: {part}");

                string key = kv[0].Trim().ToUpper();
                string value = kv[1].Trim();

                if (!float.TryParse(value, NumberStyles.Any, CultureInfo.InvariantCulture, out float parsedValue))
                {
                    throw new Exception($"Invalid number: {value}");
                }

                switch (key)
                {
                    case "THERMISTOR":
                        data.Temperature = parsedValue;
                        hasTherm = true;
                        break;

                    case "LASER":
                        data.LaserDistance = parsedValue;
                        hasLaser = true;
                        break;

                    case "POTENTIOMETER":
                        data.Angle = parsedValue;
                        hasPot = true;
                        break;

                    case "ULTRASONIC":
                        data.UltrasonicDistance = parsedValue;
                        hasUltra = true;
                        break;
                }
            }

            if (!hasTherm || !hasLaser || !hasPot || !hasUltra)
                throw new Exception("Missing sensor data");

            return data;
        }

        public void Connect(string port)
        {
            _serial.Connect(port);
        }

        public void Disconnect()
        {
            _serial.Disconnect();
        }

        // Phase 5 APIs
        public void SetCalibration(string sensor, float a, float b)
        {
            _calibService.SetCalibration(sensor, a, b);
            _commandService.SendCalibration(sensor, a, b);
        }

        public void RequestRaw(string sensor)
        {
            _commandService.RequestRaw(sensor);
        }
    }
}