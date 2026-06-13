namespace MeasurementSystem.GUI
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea5 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend5 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series5 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea6 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend6 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series6 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea7 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend7 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series7 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea8 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend8 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series8 = new System.Windows.Forms.DataVisualization.Charting.Series();
            lblTemperature = new Label();
            lblLaser = new Label();
            lblAngle = new Label();
            lblUltrasonic = new Label();
            groupBox1 = new GroupBox();
            label3 = new Label();
            groupBox2 = new GroupBox();
            label4 = new Label();
            groupBox3 = new GroupBox();
            label2 = new Label();
            groupBox4 = new GroupBox();
            label = new Label();
            lblStatus = new Label();
            chartTemp = new System.Windows.Forms.DataVisualization.Charting.Chart();
            chartLaser = new System.Windows.Forms.DataVisualization.Charting.Chart();
            chartAngle = new System.Windows.Forms.DataVisualization.Charting.Chart();
            chartUltra = new System.Windows.Forms.DataVisualization.Charting.Chart();
            btnDisconnect = new Button();
            btnReadRaw = new Button();
            btnSavePoint = new Button();
            HandleCalculateAndSend = new Button();
            lblRawADC = new Label();
            txtRealInput = new TextBox();
            label5 = new Label();
            lblPointCount = new Label();
            label7 = new Label();
            label8 = new Label();
            txtA = new TextBox();
            txtB = new TextBox();
            groupBox5 = new GroupBox();
            groupBox6 = new GroupBox();
            cboPorts = new ComboBox();
            btnConnect = new Button();
            label1 = new Label();
            groupBox1.SuspendLayout();
            groupBox2.SuspendLayout();
            groupBox3.SuspendLayout();
            groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)chartTemp).BeginInit();
            ((System.ComponentModel.ISupportInitialize)chartLaser).BeginInit();
            ((System.ComponentModel.ISupportInitialize)chartAngle).BeginInit();
            ((System.ComponentModel.ISupportInitialize)chartUltra).BeginInit();
            groupBox5.SuspendLayout();
            groupBox6.SuspendLayout();
            SuspendLayout();
            // 
            // lblTemperature
            // 
            lblTemperature.AutoSize = true;
            lblTemperature.Font = new Font("Segoe UI", 12F);
            lblTemperature.Location = new Point(32, 64);
            lblTemperature.Name = "lblTemperature";
            lblTemperature.Size = new Size(53, 38);
            lblTemperature.TabIndex = 4;
            lblTemperature.Text = "0.0";
            lblTemperature.Click += label1_Click;
            // 
            // lblLaser
            // 
            lblLaser.AutoSize = true;
            lblLaser.Font = new Font("Segoe UI", 12F);
            lblLaser.Location = new Point(33, 65);
            lblLaser.Name = "lblLaser";
            lblLaser.Size = new Size(53, 38);
            lblLaser.TabIndex = 5;
            lblLaser.Text = "0.0";
            lblLaser.Click += label5_Click;
            // 
            // lblAngle
            // 
            lblAngle.AutoSize = true;
            lblAngle.Font = new Font("Segoe UI", 12F);
            lblAngle.Location = new Point(32, 74);
            lblAngle.Name = "lblAngle";
            lblAngle.Size = new Size(53, 38);
            lblAngle.TabIndex = 6;
            lblAngle.Text = "0.0";
            // 
            // lblUltrasonic
            // 
            lblUltrasonic.AutoSize = true;
            lblUltrasonic.Font = new Font("Segoe UI", 12F);
            lblUltrasonic.Location = new Point(30, 64);
            lblUltrasonic.Name = "lblUltrasonic";
            lblUltrasonic.Size = new Size(53, 38);
            lblUltrasonic.TabIndex = 7;
            lblUltrasonic.Text = "0.0";
            lblUltrasonic.Click += label7_Click;
            // 
            // groupBox1
            // 
            groupBox1.Controls.Add(lblAngle);
            groupBox1.Controls.Add(label3);
            groupBox1.Location = new Point(495, 66);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new Size(197, 124);
            groupBox1.TabIndex = 8;
            groupBox1.TabStop = false;
            groupBox1.Text = "Góc quay";
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Font = new Font("Segoe UI", 12F);
            label3.Location = new Point(32, 35);
            label3.Name = "label3";
            label3.Size = new Size(133, 38);
            label3.TabIndex = 2;
            label3.Text = "Góc quay";
            label3.Click += label3_Click;
            // 
            // groupBox2
            // 
            groupBox2.Controls.Add(lblUltrasonic);
            groupBox2.Controls.Add(label4);
            groupBox2.Location = new Point(716, 206);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new Size(309, 111);
            groupBox2.TabIndex = 9;
            groupBox2.TabStop = false;
            groupBox2.Text = "Khoảng cách siêu âm";
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Font = new Font("Segoe UI", 12F);
            label4.Location = new Point(30, 26);
            label4.Name = "label4";
            label4.Size = new Size(279, 38);
            label4.TabIndex = 3;
            label4.Text = "Khoảng cách siêu âm";
            label4.Click += label4_Click;
            // 
            // groupBox3
            // 
            groupBox3.Controls.Add(lblLaser);
            groupBox3.Controls.Add(label2);
            groupBox3.Location = new Point(716, 66);
            groupBox3.Name = "groupBox3";
            groupBox3.Size = new Size(309, 124);
            groupBox3.TabIndex = 10;
            groupBox3.TabStop = false;
            groupBox3.Text = "Khoảng cách Laser";
            groupBox3.Enter += groupBox3_Enter;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Font = new Font("Segoe UI", 12F);
            label2.Location = new Point(33, 26);
            label2.Name = "label2";
            label2.Size = new Size(247, 38);
            label2.TabIndex = 1;
            label2.Text = "Khoảng cách Laser";
            // 
            // groupBox4
            // 
            groupBox4.Controls.Add(lblTemperature);
            groupBox4.Controls.Add(label);
            groupBox4.Location = new Point(495, 206);
            groupBox4.Name = "groupBox4";
            groupBox4.Size = new Size(197, 111);
            groupBox4.TabIndex = 11;
            groupBox4.TabStop = false;
            groupBox4.Text = "Nhiệt độ";
            groupBox4.Enter += groupBox4_Enter;
            // 
            // label
            // 
            label.AutoSize = true;
            label.Font = new Font("Segoe UI", 12F);
            label.Location = new Point(32, 25);
            label.Name = "label";
            label.Size = new Size(125, 38);
            label.TabIndex = 0;
            label.Text = "Nhiệt độ";
            label.Click += label_Click;
            // 
            // lblStatus
            // 
            lblStatus.AutoSize = true;
            lblStatus.Font = new Font("Segoe UI", 9F);
            lblStatus.Location = new Point(12, 9);
            lblStatus.Name = "lblStatus";
            lblStatus.Size = new Size(159, 30);
            lblStatus.TabIndex = 12;
            lblStatus.Text = "Đang kiểm tra...";
            lblStatus.Click += label1_Click_1;
            // 
            // chartTemp
            // 
            chartArea5.Name = "ChartArea1";
            chartTemp.ChartAreas.Add(chartArea5);
            legend5.Name = "Legend1";
            chartTemp.Legends.Add(legend5);
            chartTemp.Location = new Point(40, 438);
            chartTemp.Name = "chartTemp";
            series5.ChartArea = "ChartArea1";
            series5.Legend = "Legend1";
            series5.Name = "Series1";
            chartTemp.Series.Add(series5);
            chartTemp.Size = new Size(784, 441);
            chartTemp.TabIndex = 13;
            chartTemp.Text = "chart1";
            // 
            // chartLaser
            // 
            chartArea6.Name = "ChartArea1";
            chartLaser.ChartAreas.Add(chartArea6);
            legend6.Name = "Legend1";
            chartLaser.Legends.Add(legend6);
            chartLaser.Location = new Point(849, 438);
            chartLaser.Name = "chartLaser";
            series6.ChartArea = "ChartArea1";
            series6.Legend = "Legend1";
            series6.Name = "Series1";
            chartLaser.Series.Add(series6);
            chartLaser.Size = new Size(784, 441);
            chartLaser.TabIndex = 14;
            chartLaser.Text = "chart1";
            // 
            // chartAngle
            // 
            chartArea7.Name = "ChartArea1";
            chartAngle.ChartAreas.Add(chartArea7);
            legend7.Name = "Legend1";
            chartAngle.Legends.Add(legend7);
            chartAngle.Location = new Point(40, 907);
            chartAngle.Name = "chartAngle";
            series7.ChartArea = "ChartArea1";
            series7.Legend = "Legend1";
            series7.Name = "Series1";
            chartAngle.Series.Add(series7);
            chartAngle.Size = new Size(784, 441);
            chartAngle.TabIndex = 15;
            chartAngle.Text = "chartAngle";
            // 
            // chartUltra
            // 
            chartArea8.Name = "ChartArea1";
            chartUltra.ChartAreas.Add(chartArea8);
            legend8.Name = "Legend1";
            chartUltra.Legends.Add(legend8);
            chartUltra.Location = new Point(849, 907);
            chartUltra.Name = "chartUltra";
            series8.ChartArea = "ChartArea1";
            series8.Legend = "Legend1";
            series8.Name = "Series1";
            chartUltra.Series.Add(series8);
            chartUltra.Size = new Size(784, 441);
            chartUltra.TabIndex = 16;
            chartUltra.Text = "chartUltra";
            // 
            // btnDisconnect
            // 
            btnDisconnect.Location = new Point(192, 83);
            btnDisconnect.Name = "btnDisconnect";
            btnDisconnect.Size = new Size(159, 40);
            btnDisconnect.TabIndex = 17;
            btnDisconnect.Text = "Disconnection";
            btnDisconnect.UseVisualStyleBackColor = true;
            // 
            // btnReadRaw
            // 
            btnReadRaw.Location = new Point(41, 97);
            btnReadRaw.Name = "btnReadRaw";
            btnReadRaw.Size = new Size(131, 40);
            btnReadRaw.TabIndex = 18;
            btnReadRaw.Text = "Read Raw";
            btnReadRaw.UseVisualStyleBackColor = true;
            // 
            // btnSavePoint
            // 
            btnSavePoint.Location = new Point(41, 149);
            btnSavePoint.Name = "btnSavePoint";
            btnSavePoint.Size = new Size(131, 40);
            btnSavePoint.TabIndex = 19;
            btnSavePoint.Text = "Save Point";
            btnSavePoint.UseVisualStyleBackColor = true;
            // 
            // HandleCalculateAndSend
            // 
            HandleCalculateAndSend.Location = new Point(41, 198);
            HandleCalculateAndSend.Name = "HandleCalculateAndSend";
            HandleCalculateAndSend.Size = new Size(131, 80);
            HandleCalculateAndSend.TabIndex = 20;
            HandleCalculateAndSend.Text = "Calculate Send";
            HandleCalculateAndSend.UseVisualStyleBackColor = true;
            HandleCalculateAndSend.Click += button4_Click;
            // 
            // lblRawADC
            // 
            lblRawADC.AutoSize = true;
            lblRawADC.Location = new Point(178, 102);
            lblRawADC.Name = "lblRawADC";
            lblRawADC.Size = new Size(122, 30);
            lblRawADC.TabIndex = 21;
            lblRawADC.Text = "Giá trị ADC ";
            lblRawADC.Click += label1_Click_2;
            // 
            // txtRealInput
            // 
            txtRealInput.Location = new Point(218, 38);
            txtRealInput.Name = "txtRealInput";
            txtRealInput.Size = new Size(159, 35);
            txtRealInput.TabIndex = 22;
            // 
            // label5
            // 
            label5.AutoSize = true;
            label5.Location = new Point(43, 43);
            label5.Name = "label5";
            label5.Size = new Size(174, 30);
            label5.TabIndex = 23;
            label5.Text = "Nhập khối lượng:";
            label5.Click += label5_Click_1;
            // 
            // lblPointCount
            // 
            lblPointCount.AutoSize = true;
            lblPointCount.Location = new Point(178, 154);
            lblPointCount.Name = "lblPointCount";
            lblPointCount.Size = new Size(149, 30);
            lblPointCount.TabIndex = 24;
            lblPointCount.Text = "Đã lưu: X điểm";
            lblPointCount.Click += label6_Click;
            // 
            // label7
            // 
            label7.AutoSize = true;
            label7.Location = new Point(178, 198);
            label7.Name = "label7";
            label7.Size = new Size(38, 30);
            label7.TabIndex = 25;
            label7.Text = "a=";
            // 
            // label8
            // 
            label8.AutoSize = true;
            label8.Location = new Point(178, 248);
            label8.Name = "label8";
            label8.Size = new Size(39, 30);
            label8.TabIndex = 26;
            label8.Text = "b=";
            // 
            // txtA
            // 
            txtA.Location = new Point(219, 193);
            txtA.Name = "txtA";
            txtA.ReadOnly = true;
            txtA.Size = new Size(158, 35);
            txtA.TabIndex = 27;
            // 
            // txtB
            // 
            txtB.Location = new Point(219, 243);
            txtB.Name = "txtB";
            txtB.ReadOnly = true;
            txtB.Size = new Size(162, 35);
            txtB.TabIndex = 28;
            // 
            // groupBox5
            // 
            groupBox5.Controls.Add(txtB);
            groupBox5.Controls.Add(txtA);
            groupBox5.Controls.Add(label8);
            groupBox5.Controls.Add(label7);
            groupBox5.Controls.Add(lblPointCount);
            groupBox5.Controls.Add(label5);
            groupBox5.Controls.Add(txtRealInput);
            groupBox5.Controls.Add(lblRawADC);
            groupBox5.Controls.Add(HandleCalculateAndSend);
            groupBox5.Controls.Add(btnSavePoint);
            groupBox5.Controls.Add(btnReadRaw);
            groupBox5.Location = new Point(1119, 66);
            groupBox5.Name = "groupBox5";
            groupBox5.Size = new Size(438, 300);
            groupBox5.TabIndex = 29;
            groupBox5.TabStop = false;
            groupBox5.Text = "Calibration";
            // 
            // groupBox6
            // 
            groupBox6.Controls.Add(label1);
            groupBox6.Controls.Add(cboPorts);
            groupBox6.Controls.Add(btnConnect);
            groupBox6.Controls.Add(btnDisconnect);
            groupBox6.Location = new Point(55, 66);
            groupBox6.Name = "groupBox6";
            groupBox6.Size = new Size(369, 137);
            groupBox6.TabIndex = 30;
            groupBox6.TabStop = false;
            groupBox6.Text = "Connection";
            // 
            // cboPorts
            // 
            cboPorts.FormattingEnabled = true;
            cboPorts.Location = new Point(143, 38);
            cboPorts.Name = "cboPorts";
            cboPorts.Size = new Size(208, 38);
            cboPorts.TabIndex = 31;
            // 
            // btnConnect
            // 
            btnConnect.Location = new Point(27, 83);
            btnConnect.Name = "btnConnect";
            btnConnect.Size = new Size(159, 40);
            btnConnect.TabIndex = 31;
            btnConnect.Text = "Connection";
            btnConnect.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(27, 41);
            label1.Name = "label1";
            label1.Size = new Size(110, 30);
            label1.TabIndex = 31;
            label1.Text = "Chọn Port:";
            label1.Click += label1_Click_3;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(12F, 30F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1743, 1764);
            Controls.Add(groupBox6);
            Controls.Add(groupBox5);
            Controls.Add(chartUltra);
            Controls.Add(lblStatus);
            Controls.Add(chartAngle);
            Controls.Add(chartLaser);
            Controls.Add(chartTemp);
            Controls.Add(groupBox4);
            Controls.Add(groupBox3);
            Controls.Add(groupBox2);
            Controls.Add(groupBox1);
            Name = "Form1";
            Text = "Form1";
            Load += Form1_Load;
            groupBox1.ResumeLayout(false);
            groupBox1.PerformLayout();
            groupBox2.ResumeLayout(false);
            groupBox2.PerformLayout();
            groupBox3.ResumeLayout(false);
            groupBox3.PerformLayout();
            groupBox4.ResumeLayout(false);
            groupBox4.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)chartTemp).EndInit();
            ((System.ComponentModel.ISupportInitialize)chartLaser).EndInit();
            ((System.ComponentModel.ISupportInitialize)chartAngle).EndInit();
            ((System.ComponentModel.ISupportInitialize)chartUltra).EndInit();
            groupBox5.ResumeLayout(false);
            groupBox5.PerformLayout();
            groupBox6.ResumeLayout(false);
            groupBox6.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion
        private Label lblTemperature;
        private Label lblLaser;
        private Label lblAngle;
        private Label lblUltrasonic;
        private GroupBox groupBox1;
        private GroupBox groupBox2;
        private GroupBox groupBox3;
        private GroupBox groupBox4;
        private Label lblStatus;
        private System.Windows.Forms.DataVisualization.Charting.Chart chartTemp;
        private System.Windows.Forms.DataVisualization.Charting.Chart chartLaser;
        private System.Windows.Forms.DataVisualization.Charting.Chart chartAngle;
        private System.Windows.Forms.DataVisualization.Charting.Chart chartUltra;
        private Button btnDisconnect;
        private Button btnReadRaw;
        private Button btnSavePoint;
        private Button HandleCalculateAndSend;
        private Label lblRawADC;
        private TextBox txtRealInput;
        private Label label5;
        private Label lblPointCount;
        private Label label7;
        private Label label8;
        private TextBox txtA;
        private TextBox txtB;
        private Label label3;
        private Label label4;
        private Label label2;
        private Label label;
        private GroupBox groupBox5;
        private GroupBox groupBox6;
        private Button btnConnect;
        private ComboBox cboPorts;
        private Label label1;
    }
}
