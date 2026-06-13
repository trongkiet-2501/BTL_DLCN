namespace MeasurementSystem.Backend.Models
{
    public class Calibration
    {
        public float A { get; set; }
        public float B { get; set; }

        public float Apply(float raw)
        {
            return A * raw + B;
        }
    }
}