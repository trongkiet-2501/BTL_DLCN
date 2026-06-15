
import sys
import serial
import threading
from collections import deque

from PyQt5.QtWidgets import *
from PyQt5.QtCore import QObject, pyqtSignal
import pyqtgraph as pg


class SerialManager(QObject):
    line_received = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.ser = None
        self.running = False

    def connect_port(self, port, baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.running = True
        threading.Thread(target=self.read_thread, daemon=True).start()

    def disconnect_port(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()

    def send(self, msg):
        if self.ser and self.ser.is_open:
            self.ser.write((msg + "\n").encode())

    def read_thread(self):
        while self.running:
            try:
                line = self.ser.readline().decode(errors="ignore").strip()
                if line:
                    self.line_received.emit(line)
            except Exception:
                pass


class SensorGUI(QWidget):

    sensors = ["LASER", "THERMISTOR", "POTENTIOMETER", "ULTRASONIC"]

    def __init__(self):
        super().__init__()

        self.serial = SerialManager()
        self.serial.line_received.connect(self.process_line)

        self.calibrated = {s: False for s in self.sensors}

        self.sample_index = 0
        self.x_data = deque(maxlen=500)
        self.sensor_data = {s: deque(maxlen=500) for s in self.sensors}

        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("STM32 Sensor Calibration GUI")
        self.resize(1400, 800)

        main_layout = QHBoxLayout()

        left = QVBoxLayout()

        # Serial
        serial_group = QGroupBox("UART TTL")
        sl = QHBoxLayout()

        self.port_box = QComboBox()
        self.refresh_ports()

        btn_refresh = QPushButton("Refresh")
        btn_refresh.clicked.connect(self.refresh_ports)

        btn_connect = QPushButton("Connect")
        btn_connect.clicked.connect(self.connect_serial)

        btn_disconnect = QPushButton("Disconnect")
        btn_disconnect.clicked.connect(self.disconnect_serial)

        self.status_label = QLabel("Disconnected")
        self.status_label.setStyleSheet("color:red;font-weight:bold;")

        sl.addWidget(QLabel("Port"))
        sl.addWidget(self.port_box)
        sl.addWidget(btn_refresh)
        sl.addWidget(btn_connect)
        sl.addWidget(btn_disconnect)
        sl.addWidget(QLabel("Status:"))
        sl.addWidget(self.status_label)

        serial_group.setLayout(sl)
        left.addWidget(serial_group)

        # Calibration
        calib_group = QGroupBox("Calibration")
        grid = QGridLayout()

        headers = ["Sensor", "Point1", "", "Point2", "", "Status"]
        for c, h in enumerate(headers):
            grid.addWidget(QLabel(h), 0, c)

        self.p1_boxes = {}
        self.p2_boxes = {}
        self.calib_labels = {}

        for row, sensor in enumerate(self.sensors, start=1):

            p1 = QLineEdit()
            p2 = QLineEdit()

            self.p1_boxes[sensor] = p1
            self.p2_boxes[sensor] = p2

            status = QLabel("Not Calibrated")
            status.setStyleSheet("color:red;")
            self.calib_labels[sensor] = status

            btn1 = QPushButton("Set P1")
            btn2 = QPushButton("Set P2")

            btn1.clicked.connect(lambda _, s=sensor: self.send_p1(s))
            btn2.clicked.connect(lambda _, s=sensor: self.send_p2(s))

            grid.addWidget(QLabel(sensor), row, 0)
            grid.addWidget(p1, row, 1)
            grid.addWidget(btn1, row, 2)
            grid.addWidget(p2, row, 3)
            grid.addWidget(btn2, row, 4)
            grid.addWidget(status, row, 5)

        calib_group.setLayout(grid)
        left.addWidget(calib_group)

        # Calibration Parameters
        param_group = QGroupBox("Calibration Parameters")

        pg_layout = QGridLayout()

        self.a_labels = {}
        self.b_labels = {}

        for row, sensor in enumerate(self.sensors):

            pg_layout.addWidget(QLabel(sensor), row, 0)

            a_lbl = QLabel("A = ---")
            b_lbl = QLabel("B = ---")

            self.a_labels[sensor] = a_lbl
            self.b_labels[sensor] = b_lbl

            pg_layout.addWidget(a_lbl, row, 1)
            pg_layout.addWidget(b_lbl, row, 2)

        param_group.setLayout(pg_layout)

        left.addWidget(param_group)

        # Current values
        values_group = QGroupBox("Current Values")
        vg = QGridLayout()

        self.value_labels = {}

        for r, sensor in enumerate(self.sensors):
            vg.addWidget(QLabel(sensor), r, 0)

            lbl = QLabel("0.00")
            self.value_labels[sensor] = lbl
            vg.addWidget(lbl, r, 1)

        values_group.setLayout(vg)
        left.addWidget(values_group)

        # Data control
        data_group = QGroupBox("Data Control")
        dl = QHBoxLayout()

        btn_start = QPushButton("Start Reading")
        btn_stop = QPushButton("Stop Reading")

        btn_start.clicked.connect(self.start_reading)
        btn_stop.clicked.connect(self.stop_reading)

        dl.addWidget(btn_start)
        dl.addWidget(btn_stop)

        data_group.setLayout(dl)
        left.addWidget(data_group)

        # Log
        self.log_box = QTextEdit()
        self.log_box.setReadOnly(True)
        left.addWidget(self.log_box)

        # Plot
        self.plot = pg.PlotWidget()
        self.plot.addLegend()
        self.plot.setLabel("left", "Value")
        self.plot.setLabel("bottom", "Sample")

        colors = {
            "LASER": (255, 0, 0),
            "THERMISTOR": (0, 255, 0),
            "POTENTIOMETER": (0, 100, 255),
            "ULTRASONIC": (255, 200, 0)
        }

        self.curves = {}

        for sensor in self.sensors:
            self.curves[sensor] = self.plot.plot(
                [], [],
                pen=pg.mkPen(color=colors[sensor], width=2),
                name=sensor
            )

        main_layout.addLayout(left, 1)
        main_layout.addWidget(self.plot, 2)

        self.setLayout(main_layout)

    def refresh_ports(self):
        try:
            import serial.tools.list_ports
            self.port_box.clear()
            for p in serial.tools.list_ports.comports():
                self.port_box.addItem(p.device)
        except Exception:
            pass

    def connect_serial(self):
        try:
            port = self.port_box.currentText()
            self.serial.connect_port(port)

            self.status_label.setText("Connected")
            self.status_label.setStyleSheet("color:green;font-weight:bold;")

            self.log("Connected: " + port)
        except Exception as e:
            QMessageBox.warning(self, "Error", str(e))

    def disconnect_serial(self):
        self.serial.disconnect_port()

        self.status_label.setText("Disconnected")
        self.status_label.setStyleSheet("color:red;font-weight:bold;")

        self.log("Disconnected")

    def log(self, text):
        self.log_box.append(text)

    def send_p1(self, sensor):
        value = self.p1_boxes[sensor].text().strip()
        cmd = f"SETP1,{sensor},{value}"
        self.serial.send(cmd)
        self.log("TX > " + cmd)

    def send_p2(self, sensor):
        value = self.p2_boxes[sensor].text().strip()
        cmd = f"SETP2,{sensor},{value}"
        self.serial.send(cmd)
        self.log("TX > " + cmd)

    def start_reading(self):
        self.serial.send("START")
        self.log("TX > START")

    def stop_reading(self):
        self.serial.send("STOP")
        self.log("TX > STOP")

    def process_line(self, line):

        self.log("RX > " + line)

        if line.startswith("OK,SETP2"):
            try:

                parts = line.split(",")

                sensor = parts[2]

                a_value = None
                b_value = None

                for p in parts:

                    if p.startswith("A:"):
                        a_value = p[2:]

                    elif p.startswith("B:"):
                        b_value = p[2:]

                if sensor in self.sensors:

                    self.calibrated[sensor] = True

                    self.calib_labels[sensor].setText(
                        "Calibrated"
                    )

                    self.calib_labels[sensor].setStyleSheet(
                        "color:green;"
                    )

                    self.a_labels[sensor].setText(
                        f"A = {a_value}"
                    )

                    self.b_labels[sensor].setText(
                        f"B = {b_value}"
                    )

            except Exception as e:
                    self.log(f"Parse calibration error: {e}")

            return

        if line.startswith("CALIB_OK:"):
            sensor = line.split(":")[1]

            if sensor in self.calibrated:
                self.calibrated[sensor] = True

                self.calib_labels[sensor].setText("Calibrated")
                self.calib_labels[sensor].setStyleSheet("color:green;")

            return

        try:
            data = {}

            for item in line.split(";"):
                if ":" not in item:
                    continue

                k, v = item.split(":")
                data[k.strip()] = float(v)

            self.update_sensor_data(data)

        except Exception:
            pass

    def update_sensor_data(self, data):

        self.sample_index += 1
        self.x_data.append(self.sample_index)
        for sensor in self.sensors:

            if sensor not in data:
                continue

            self.value_labels[sensor].setText(
                f"{data[sensor]:.2f}"
            )

            # Tự động đánh dấu đã calib
            if not self.calibrated[sensor]:

                self.calibrated[sensor] = True

                self.calib_labels[sensor].setText(
                    "Calibrated"
                )

                self.calib_labels[sensor].setStyleSheet(
                    "color:green;"
                )

            self.sensor_data[sensor].append(data[sensor])

            x = list(self.x_data)
            y = list(self.sensor_data[sensor])

            min_len = min(len(x), len(y))

            self.curves[sensor].setData(
                x[-min_len:],
                y[-min_len:]
            )
            if len(self.x_data) > 50:
                self.plot.setXRange(
                    self.x_data[-50],
                    self.x_data[-1],
                    padding=0
                )


if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = SensorGUI()
    win.show()
    sys.exit(app.exec_())
