using MeasurementSystem.Backend.Services;
using MeasurementSystem.Backend.Models;
using System;

namespace MeasurementSystem.GUI.Services
{
    public class UIManager
    {
        private DataService _dataService;
        private SerialService _serialService; 

        public event Action<SensorData> OnUiRefreshRequired;
        public event Action<bool> OnConnectionStatusChanged; // Chuông báo trạng thái kết nối

        public UIManager()
        {
            // 1. Khởi tạo dịch vụ Serial trước
            _serialService = new SerialService();

            // 2. Truyền Serial vào DataService
            _dataService = new DataService(_serialService);

            // Đăng ký nhận dữ liệu
            _dataService.OnDataUpdated += (data) => OnUiRefreshRequired?.Invoke(data);

            // Đăng ký nhận trạng thái kết nối
            _serialService.OnDisconnected += () => OnConnectionStatusChanged?.Invoke(false);
        }

        public void SetCalibration(string sensor, float a, float b)
        {
            // Gọi xuống DataService của Backend để thực hiện calib và gửi lệnh xuống MCU
            _dataService.SetCalibration(sensor, a, b);
        }

        public void RequestRaw(string sensor)
        {
            _dataService.RequestRaw(sensor);
        }

        public void ConnectDevice(string portName)
        {
            try
            {
                _dataService.Connect(portName);
                // Nếu không có lỗi xảy ra, báo trạng thái kết nối thành công
                OnConnectionStatusChanged?.Invoke(true);
            }
            catch (Exception)
            {
                OnConnectionStatusChanged?.Invoke(false);
            }
        }

        public void DisconnectDevice()
        {
            _dataService.Disconnect();
            OnConnectionStatusChanged?.Invoke(false);
        }
    }
}