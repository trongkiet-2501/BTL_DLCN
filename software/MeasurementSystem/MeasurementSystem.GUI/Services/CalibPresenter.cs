using System;
using System.Windows.Forms;

namespace MeasurementSystem.GUI.Services
{
    public class CalibPresenter
    {
        private CalibLogic _logic;
        private UIManager _ui;

        public CalibPresenter(UIManager ui)
        {
            _logic = new CalibLogic();
            _ui = ui;
        }

        // Xử lý logic cho nút Save Point
        public void HandleSavePoint(string rawText, string realText, Label lblStatus)
        {
            if (float.TryParse(rawText, out float raw) && float.TryParse(realText, out float real))
            {
                _logic.SavePoint(raw, real);
                lblStatus.Text = $"Đã lưu: {_logic.PointCount} điểm";
            }
            else
            {
                MessageBox.Show("Dữ liệu nhập vào không hợp lệ!");
            }
        }

        // Xử lý logic cho nút Calculate & Send
        public void HandleCalculateAndSend(TextBox txtA, TextBox txtB)
        {
            try
            {
                var result = _logic.Calculate();

                txtA.Text = result.a.ToString("F4");
                txtB.Text = result.b.ToString("F4");

                // Gửi xuống hệ thống
                _ui.SetCalibration("LOAD", result.a, result.b);

                MessageBox.Show("Đã tính toán và gửi thông số Calib thành công!");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public void HandleConnect(string portName)
        {
            if (string.IsNullOrEmpty(portName))
            {
                MessageBox.Show("Vui lòng chọn cổng COM!");
                return;
            }
            _ui.ConnectDevice(portName); // Gọi xuống UIManager đã có hàm này
            MessageBox.Show($"Đã kết nối tới {portName}");
        }

        public void HandleDisconnect()
        {
            _ui.DisconnectDevice(); // Gọi xuống UIManager
            MessageBox.Show(" đã ngắt kết nối.");
        }
    }
}