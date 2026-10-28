"""Live PID tuner for the self-balancing robot.

Sends "PID:kp,ki,kd\n" over serial (115200 baud) to the ESP32,
which updates its gains on the fly. Requires: pip install pyserial
"""
import tkinter as tk
from tkinter import ttk
import serial
import serial.tools.list_ports

DEFAULTS = {"kp": 58000.0, "ki": 1.2, "kd": 500.0}


class PIDTuner:
    def __init__(self, root):
        self.root = root
        root.title("Self-Balancing Robot - PID Tuner")
        self.ser = None
        self.vars = {k: tk.DoubleVar(value=v) for k, v in DEFAULTS.items()}

        frame = ttk.Frame(root, padding=10)
        frame.grid(row=0, column=0)

        limits = {"kp": (0, 100000, 100), "ki": (0, 50, 0.1), "kd": (0, 20000, 10)}
        for row, name in enumerate(("kp", "ki", "kd")):
            lo, hi, step = limits[name]
            ttk.Label(frame, text=name.upper()).grid(row=row, column=0, padx=5, pady=2)
            spin = ttk.Spinbox(frame, from_=lo, to=hi, increment=step, width=12,
                               textvariable=self.vars[name], command=self.send)
            spin.grid(row=row, column=1, padx=5)
            spin.bind("<Return>", lambda e: self.send())
            spin.bind("<FocusOut>", lambda e: self.send())

        self.port = ttk.Combobox(frame, values=self.ports(), width=14)
        self.port.set("Select port")
        self.port.grid(row=3, column=0, columnspan=2, pady=8)
        self.btn = ttk.Button(frame, text="Connect", command=self.connect)
        self.btn.grid(row=3, column=2, padx=5)
        ttk.Button(frame, text="Send", command=self.send).grid(row=4, column=0, columnspan=3)
        self.status = ttk.Label(frame, text="Not connected")
        self.status.grid(row=5, column=0, columnspan=3, pady=5)

    @staticmethod
    def ports():
        return [p.device for p in serial.tools.list_ports.comports()]

    def connect(self):
        try:
            self.ser = serial.Serial(self.port.get(), 115200, timeout=1)
            self.btn.config(text="Connected", state="disabled")
            self.status.config(text=f"Connected to {self.port.get()}")
            self.send()
        except Exception as e:
            self.status.config(text=f"Error: {e}")

    def send(self):
        if not (self.ser and self.ser.is_open):
            return
        try:
            kp, ki, kd = (self.vars[k].get() for k in ("kp", "ki", "kd"))
        except tk.TclError:
            self.status.config(text="Enter valid numbers")
            return
        msg = f"PID:{kp:.2f},{ki:.2f},{kd:.2f}\n"
        try:
            self.ser.write(msg.encode())
            self.status.config(text=f"Sent {msg.strip()}")
        except Exception as e:
            self.status.config(text=f"Write error: {e}")


if __name__ == "__main__":
    root = tk.Tk()
    PIDTuner(root)
    root.mainloop()
