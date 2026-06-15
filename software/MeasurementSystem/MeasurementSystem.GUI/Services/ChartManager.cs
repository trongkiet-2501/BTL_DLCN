using System.Windows.Forms.DataVisualization.Charting;

namespace MeasurementSystem.GUI.Services
{
    public class ChartManager
    {
        private Chart _chart;
        private string _seriesName;

        // Sửa hàm khởi tạo để nhận thêm tên Series (ví dụ: "Nhiệt độ")
        public ChartManager(Chart chart, string seriesName)
        {
            _chart = chart;
            _seriesName = seriesName;
            InitializeChart();
        }

        private void InitializeChart()
        {
            _chart.Series.Clear();
            var series = _chart.Series.Add(_seriesName);
            series.ChartType = SeriesChartType.Spline; // Vẽ đường cong mượt
            series.BorderWidth = 2;
        }

        public void AddDataPoint(float value)
        {
            if (_chart.InvokeRequired)
            {
                _chart.Invoke(new Action(() => AddDataPoint(value)));
                return;
            }

            var points = _chart.Series[_seriesName].Points;
            points.AddY(value);

            if (points.Count > 30) // Giới hạn 30 điểm
                points.RemoveAt(0);
        }
    }
}