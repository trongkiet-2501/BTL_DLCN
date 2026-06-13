public class StatusManager
{
    private Label _lblStatus;

    public StatusManager(Label lblStatus)
    {
        _lblStatus = lblStatus;
    }

    public void SetStatus(bool isConnected)
    {
        if (_lblStatus.InvokeRequired)
        {
            _lblStatus.Invoke(new Action(() => SetStatus(isConnected)));
            return;
        }

        _lblStatus.Text = isConnected ? "Trạng thái: ĐÃ KẾT NỐI" : "Trạng thái: MẤT KẾT NỐI";
        _lblStatus.ForeColor = isConnected ? Color.Green : Color.Red;
    }
}