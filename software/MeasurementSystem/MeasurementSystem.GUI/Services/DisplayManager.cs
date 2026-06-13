using MeasurementSystem.Backend.Models;

public class DisplayManager
{
    private Label _lblTemp, _lblLaser, _lblAngle, _lblUltra;

    public DisplayManager(Label temp, Label laser, Label angle, Label ultra)
    {
        _lblTemp = temp;
        _lblLaser = laser;
        _lblAngle = angle;
        _lblUltra = ultra;
    }

    public void UpdateAll(SensorData data)
    {
        // Thực hiện Invoke ngay tại đây để Form1 không phải làm nữa
        if (_lblTemp.InvokeRequired)
        {
            _lblTemp.Invoke(new Action(() => UpdateAll(data)));
            return;
        }

        _lblTemp.Text = $"{data.Temperature:0.0} °C";
        _lblLaser.Text = $"{data.LaserDistance:0.0} cm";
        _lblAngle.Text = $"{data.Angle:0.0} °";
        _lblUltra.Text = $"{data.UltrasonicDistance:0.0} cm";
    }
}