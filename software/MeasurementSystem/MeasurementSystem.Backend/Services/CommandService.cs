namespace MeasurementSystem.Backend.Services
{
    public class CommandService
    {
        private SerialService _serial;

        public CommandService(SerialService serial)
        {
            _serial = serial;
        }

        public void RequestRaw(string sensor)
        {
            _serial.Send($"GET_RAW {sensor}");
        }

        public void SendCalibration(string sensor, float a, float b)
        {
            _serial.Send($"SET_CALIB {sensor} {a} {b}");
        }
    }
}