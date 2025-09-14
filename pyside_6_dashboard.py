from PySide6 import QtCore, QtWidgets, QtGui
import sys
import random
from datetime import datetime

# -----------------------------
# Military-grade colour palette
# -----------------------------
PALETTE = {
    "bg": "#2B2D2F",            # dark background
    "topbar": "#3A5031",        # deep olive
    "active": "#6B8E23",        # olive drab (active)
    "accent": "#8B6F47",        # tan accent
    "text": "#E6E6E6",
    "health_good": "#4CAF50",
    "health_warn": "#FFC107",
    "health_bad": "#F44336",
}

STYLE = f"""
QMainWindow {{ background: {PALETTE['bg']}; color: {PALETTE['text']}; }}
QWidget {{ background: transparent; color: {PALETTE['text']}; font-family: Arial; }}
QPushButton {{
    border-radius: 6px;
    padding: 6px 10px;
    background: transparent;
    border: 1px solid #4b4b4b;
}}
QPushButton:hover {{ border: 1px solid {PALETTE['accent']}; }}
QFrame#TopBar {{ background: {PALETTE['topbar']}; padding: 6px; }}
QFrame#BottomBar {{ background: #1F1F1F; padding: 6px; }}
QLabel#Title {{ font-size: 18px; font-weight: bold; }}
QTreeWidget {{ background: #232425; border: 1px solid #3a3a3a; }}
QTableWidget {{ background: #232425; border: 1px solid #3a3a3a; }}
QLineEdit, QComboBox {{ background: #2E2E2E; border: 1px solid #444; padding: 4px; color: {PALETTE['text']}; }}
"""


class LoginWindow(QtWidgets.QDialog):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Login")
        self.setFixedSize(360, 200)
        self.setStyleSheet(STYLE)

        layout = QtWidgets.QVBoxLayout(self)

        title = QtWidgets.QLabel("Secure GUI Login")
        title.setObjectName("Title")
        title.setAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(title)

        form = QtWidgets.QFormLayout()
        self.username = QtWidgets.QLineEdit()
        self.password = QtWidgets.QLineEdit()
        self.password.setEchoMode(QtWidgets.QLineEdit.EchoMode.Password)
        form.addRow("Username:", self.username)
        form.addRow("Password:", self.password)
        layout.addLayout(form)

        btn = QtWidgets.QPushButton("Login")
        btn.clicked.connect(self.try_login)
        layout.addWidget(btn)

        self.message = QtWidgets.QLabel("")
        self.message.setAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.message)

        # Hardcoded credentials for demo. Replace with real auth.
        self._valid_user = "admin"
        self._valid_pass = "password"

    def try_login(self):
        if self.username.text() == self._valid_user and self.password.text() == self._valid_pass:
            self.accept()
        else:
            self.message.setText("Invalid credentials")


class HomePage(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        layout = QtWidgets.QVBoxLayout(self)

        # Center: hierarchy tree where each name is a button inside the tree
        tree = QtWidgets.QTreeWidget()
        tree.setHeaderHidden(True)
        tree.setIndentation(16)

        # Example hierarchy
        roots = {
            "Power System": ["Battery", "Converter", "Distributor"],
            "Propulsion": ["Main Motor", "Thruster A", "Thruster B"],
            "Comms": ["Antenna", "Radio", "Modem"],
        }

        for rname, children in roots.items():
            ritem = QtWidgets.QTreeWidgetItem(tree)
            rbtn = QtWidgets.QPushButton(rname)
            rbtn.clicked.connect(lambda checked, n=rname: self.on_item_clicked(n))
            tree.addTopLevelItem(ritem)
            tree.setItemWidget(ritem, 0, rbtn)

            for cname in children:
                citem = QtWidgets.QTreeWidgetItem(ritem)
                cbtn = QtWidgets.QPushButton(cname)
                cbtn.clicked.connect(lambda checked, n=cname: self.on_item_clicked(n))
                ritem.addChild(citem)
                tree.setItemWidget(citem, 0, cbtn)

        layout.addWidget(tree)

    def on_item_clicked(self, name):
        QtWidgets.QMessageBox.information(self, "Item Clicked", f"You clicked: {name}")


class SensorsPage(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        main = QtWidgets.QHBoxLayout(self)

        # Left vertical 7 buttons
        left_frame = QtWidgets.QFrame()
        left_frame.setFixedWidth(160)
        left_layout = QtWidgets.QVBoxLayout(left_frame)
        left_layout.setAlignment(QtCore.Qt.AlignmentFlag.AlignTop)

        self.sensor_buttons = []
        for i in range(1, 8):
            b = QtWidgets.QPushButton(f"Sensor {i}")
            b.setCheckable(True)
            b.clicked.connect(self.highlight_sensor)
            left_layout.addWidget(b)
            self.sensor_buttons.append(b)

        main.addWidget(left_frame)

        # Right: placeholder for sensor details
        self.detail = QtWidgets.QLabel("Select a sensor to view details")
        self.detail.setAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
        main.addWidget(self.detail, 1)

    def highlight_sensor(self):
        sender = self.sender()
        for b in self.sensor_buttons:
            b.setChecked(False)
            b.setStyleSheet("")
        sender.setChecked(True)
        sender.setStyleSheet(f"background: {PALETTE['active']}; color: black; font-weight: bold;")
        self.detail.setText(f"Details for {sender.text()}")


class FaultsPage(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        layout = QtWidgets.QHBoxLayout(self)

        # Active faults
        active_group = QtWidgets.QGroupBox("Active Faults")
        a_layout = QtWidgets.QVBoxLayout(active_group)
        self.active_table = QtWidgets.QTableWidget(0, 4)
        self.active_table.setHorizontalHeaderLabels(["ID", "Severity", "Location", "Time"])
        a_layout.addWidget(self.active_table)

        # History
        hist_group = QtWidgets.QGroupBox("Fault History")
        h_layout = QtWidgets.QVBoxLayout(hist_group)
        self.hist_table = QtWidgets.QTableWidget(0, 4)
        self.hist_table.setHorizontalHeaderLabels(["ID", "Severity", "Location", "Time"])
        h_layout.addWidget(self.hist_table)

        splitter = QtWidgets.QSplitter(QtCore.Qt.Orientation.Horizontal)
        splitter.addWidget(active_group)
        splitter.addWidget(hist_group)
        layout.addWidget(splitter)

        # Add sample data
        self.add_active_fault("F001", "Critical", "Power/Batt", "2025-09-11 10:00:00")
        self.add_active_fault("F002", "Warning", "Comms/Modem", "2025-09-11 09:50:00")
        self.add_history_fault("H101", "Info", "Maintenance", "2025-08-30 14:12:00")

    def add_active_fault(self, fid, severity, loc, time):
        r = self.active_table.rowCount()
        self.active_table.insertRow(r)
        for c, v in enumerate([fid, severity, loc, time]):
            self.active_table.setItem(r, c, QtWidgets.QTableWidgetItem(v))

    def add_history_fault(self, fid, severity, loc, time):
        r = self.hist_table.rowCount()
        self.hist_table.insertRow(r)
        for c, v in enumerate([fid, severity, loc, time]):
            self.hist_table.setItem(r, c, QtWidgets.QTableWidgetItem(v))


class SettingsPage(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        layout = QtWidgets.QFormLayout(self)

        # Change username
        self.user_edit = QtWidgets.QLineEdit()
        self.user_apply = QtWidgets.QPushButton("Change Username")
        self.user_apply.clicked.connect(self.change_username)
        layout.addRow("New username:", self.user_edit)
        layout.addRow("", self.user_apply)

        # Change password
        self.old_pass = QtWidgets.QLineEdit()
        self.old_pass.setEchoMode(QtWidgets.QLineEdit.EchoMode.Password)
        self.new_pass = QtWidgets.QLineEdit()
        self.new_pass.setEchoMode(QtWidgets.QLineEdit.EchoMode.Password)
        self.pass_apply = QtWidgets.QPushButton("Change Password")
        self.pass_apply.clicked.connect(self.change_password)
        layout.addRow("Old password:", self.old_pass)
        layout.addRow("New password:", self.new_pass)
        layout.addRow("", self.pass_apply)

        # Password access toggle
        self.pw_access = QtWidgets.QCheckBox("Require password for critical ops")
        layout.addRow(self.pw_access)

        # Other elements
        self.save_btn = QtWidgets.QPushButton("Save Settings")
        layout.addRow("", self.save_btn)

    def change_username(self):
        new = self.user_edit.text()
        QtWidgets.QMessageBox.information(self, "Username", f"Username changed to: {new}")

    def change_password(self):
        QtWidgets.QMessageBox.information(self, "Password", "Password changed (demo)")


class AcquisitionPage(QtWidgets.QWidget):
    """Common acquisition page used for STM and RaspberryPi, with slightly different labels"""
    def __init__(self, device_name="Device"):
        super().__init__()
        self.device_name = device_name
        layout = QtWidgets.QFormLayout(self)

        self.addr = QtWidgets.QLineEdit()
        self.port = QtWidgets.QLineEdit()
        self.connect_btn = QtWidgets.QPushButton("Connect")
        self.disconnect_btn = QtWidgets.QPushButton("Disconnect")
        self.start_btn = QtWidgets.QPushButton("Start Acquisition")
        self.stop_btn = QtWidgets.QPushButton("Stop Acquisition")

        btn_row = QtWidgets.QHBoxLayout()
        btn_row.addWidget(self.connect_btn)
        btn_row.addWidget(self.disconnect_btn)
        btn_row.addWidget(self.start_btn)
        btn_row.addWidget(self.stop_btn)

        layout.addRow(f"{device_name} Address/Port:", self.addr)
        layout.addRow("Port/Serial:", self.port)
        layout.addRow("", btn_row)

        self.status = QtWidgets.QLabel("Disconnected")
        layout.addRow("Status:", self.status)

        # Connect slots
        self.connect_btn.clicked.connect(lambda: self.set_status("Connected"))
        self.disconnect_btn.clicked.connect(lambda: self.set_status("Disconnected"))
        self.start_btn.clicked.connect(lambda: self.set_status("Acquiring"))
        self.stop_btn.clicked.connect(lambda: self.set_status("Idle"))

    def set_status(self, s: str):
        self.status.setText(f"{self.device_name}: {s}")


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Control GUI")
        self.resize(1100, 700)
        self.setStyleSheet(STYLE)

        # Central widget and layout
        central = QtWidgets.QWidget()
        central_layout = QtWidgets.QVBoxLayout(central)
        central_layout.setContentsMargins(0, 0, 0, 0)
        central_layout.setSpacing(0)

        # Top bar (frozen)
        self.topbar = QtWidgets.QFrame()
        self.topbar.setObjectName("TopBar")
        tlay = QtWidgets.QHBoxLayout(self.topbar)
        tlay.setAlignment(QtCore.Qt.AlignmentFlag.AlignLeft)

        # Top buttons
        self.top_buttons = {}
        names = ["Home", "Sensors", "STM", "RaspberryPi", "Faults", "Settings"]
        for name in names:
            b = QtWidgets.QPushButton(name)
            b.setCheckable(True)
            b.clicked.connect(lambda checked, n=name: self.go_to(n))
            self.top_buttons[name] = b
            tlay.addWidget(b)

        tlay.addStretch()
        central_layout.addWidget(self.topbar)

        # Stacked pages
        self.stack = QtWidgets.QStackedWidget()
        self.pages = {
            "Home": HomePage(),
            "Sensors": SensorsPage(),
            "STM": AcquisitionPage("STM"),
            "RaspberryPi": AcquisitionPage("RaspberryPi"),
            "Faults": FaultsPage(),
            "Settings": SettingsPage(),
        }
        for p in names:
            self.stack.addWidget(self.pages[p])

        central_layout.addWidget(self.stack, 1)

        # Bottom bar (frozen)
        self.bottombar = QtWidgets.QFrame()
        self.bottombar.setObjectName("BottomBar")
        blay = QtWidgets.QHBoxLayout(self.bottombar)

        # Back/Next buttons (hidden on home)
        self.back_btn = QtWidgets.QPushButton("Back")
        self.next_btn = QtWidgets.QPushButton("Next")
        self.back_btn.clicked.connect(self.go_back)
        self.next_btn.clicked.connect(self.go_next)

        blay.addWidget(self.back_btn)
        blay.addWidget(self.next_btn)

        # Running window showing system health
        self.health_label = QtWidgets.QLabel("System Health: Initializing...")
        blay.addStretch()
        blay.addWidget(self.health_label)

        central_layout.addWidget(self.bottombar)

        self.setCentralWidget(central)

        # Navigation history
        self.history = []
        self.forward = []

        # Default to Home
        self.go_to("Home", add_history=False)

        # Start timer to update health
        self._health_state = 0
        self.health_timer = QtCore.QTimer(self)
        self.health_timer.timeout.connect(self.update_health)
        self.health_timer.start(1000)

    def update_health(self):
        # Simple simulated health alternating logic
        self._health_state = (self._health_state + 1) % 10
        if self._health_state < 7:
            status = "HEALTHY"
            color = PALETTE['health_good']
        elif self._health_state < 9:
            status = "WARN"
            color = PALETTE['health_warn']
        else:
            status = "CRITICAL"
            color = PALETTE['health_bad']

        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.health_label.setText(f"System Health: {status} | {now}")
        self.health_label.setStyleSheet(f"font-weight:bold; color: {color};")

    def go_to(self, name: str, add_history: bool = True):
        # Navigate to page name
        index = list(self.pages.keys()).index(name)
        # Manage history stacks
        if add_history:
            if len(self.history) == 0 or self.history[-1] != index:
                self.history.append(index)
            self.forward.clear()
        self.stack.setCurrentIndex(index)

        # Highlight top button
        for n, btn in self.top_buttons.items():
            if n == name:
                btn.setChecked(True)
                btn.setStyleSheet(f"background: {PALETTE['active']}; color: black; font-weight: bold;")
            else:
                btn.setChecked(False)
                btn.setStyleSheet("")

        # Show/hide back/next depending on current (home hides them)
        if name == "Home":
            self.back_btn.hide()
            self.next_btn.hide()
        else:
            self.back_btn.setVisible(len(self.history) > 1)
            self.next_btn.setVisible(len(self.forward) > 0)

    def go_back(self):
        if len(self.history) > 1:
            cur = self.history.pop()
            self.forward.append(cur)
            prev = self.history[-1]
            self.stack.setCurrentIndex(prev)
            # update button highlight by name
            name = list(self.pages.keys())[prev]
            self.go_to(name, add_history=False)

    def go_next(self):
        if len(self.forward) > 0:
            nxt = self.forward.pop()
            self.history.append(nxt)
            name = list(self.pages.keys())[nxt]
            self.go_to(name, add_history=False)


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    app.setStyle('Fusion')

    login = LoginWindow()
    if login.exec() == QtWidgets.QDialog.Accepted:
        w = MainWindow()
        w.show()
        sys.exit(app.exec())
    else:
        sys.exit(0)
