using System;
using System.IO.Ports;

namespace MeasurementSystem.Backend.Services
{
    public class SerialService
    {
        private SerialPort _port;

        public event Action<string>? OnRawDataReceived;
        public event Action? OnDisconnected;

        public bool IsConnected => _port != null && _port.IsOpen;

        public void Connect(string portName, int baudRate = 115200)
        {
            try
            {
                _port = new SerialPort(portName, baudRate);
                _port.NewLine = "\n";
                _port.DtrEnable = true;
                _port.RtsEnable = true;
                _port.DataReceived += Port_DataReceived;
                _port.Open();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Connect error: " + ex.Message);
            }
        }

        public void Disconnect()
        {
            if (_port != null && _port.IsOpen)
            {
                _port.Close();
                OnDisconnected?.Invoke();
            }
        }

        public void Send(string message)
        {
            if (IsConnected)
            {
                try
                {
                    _port.WriteLine(message);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Send error: " + ex.Message);
                }
            }
        }

        private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string line = _port.ReadLine();
                OnRawDataReceived?.Invoke(line.Trim());
            }
            catch
            {
                OnDisconnected?.Invoke();
            }
        }
    }
}