import os
import math

def setup_dirs(base_dir):
    paths = [
        os.path.join(base_dir, "plugins", "electronics-suite", "assets", "passives"),
        os.path.join(base_dir, "plugins", "electronics-suite", "assets", "semiconductors"),
        os.path.join(base_dir, "plugins", "electronics-suite", "assets", "digital"),
        os.path.join(base_dir, "plugins", "electronics-suite", "assets", "waveforms"),
        os.path.join(base_dir, "plugins", "electronics-suite", "assets", "blocks"),
        os.path.join(base_dir, "templates", "backgrounds"),
        os.path.join(base_dir, "config")
    ]
    for p in paths:
        os.makedirs(p, exist_ok=True)
    return paths

def save_svg(path, content, width=100, height=100, viewbox=None):
    if viewbox is None:
        viewbox = f"0 0 {width} {height}"

    header = f'''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n<svg width="{width}" height="{height}" viewBox="{viewbox}" xmlns="http://www.w3.org/2000/svg">\n'''
    footer = '</svg>'
    with open(path, "w") as f:
        f.write(header + content + footer)

def line(x1, y1, x2, y2, stroke="black", width="2"):
    return f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{stroke}" stroke-width="{width}" />'

def circle(cx, cy, r, fill="none", stroke="black", width="2"):
    return f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{width}" />'

def rect(x, y, w, h, fill="none", stroke="black", width="2", rx="0", ry="0"):
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" ry="{ry}" fill="{fill}" stroke="{stroke}" stroke-width="{width}" />'

def text(x, y, content, font_size="12", font_family="sans-serif", anchor="middle"):
    return f'<text x="{x}" y="{y}" font-size="{font_size}" font-family="{font_family}" text-anchor="{anchor}">{content}</text>'

def path(d, fill="none", stroke="black", width="2"):
    return f'<path d="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{width}" />'

def gen_waveforms(base_dir):
    out_dir = os.path.join(base_dir, "plugins", "electronics-suite", "assets", "waveforms")

    # 2.1 Coordinate System
    content = line(10, 90, 190, 90) + path("M185 85 L190 90 L185 95") + \
              line(10, 90, 10, 10) + path("M5 15 L10 10 L15 15") + \
              text(190, 105, "t") + text(5, 20, "V") + text(5, 95, "0")
    save_svg(os.path.join(out_dir, "coord_t_V.svg"), content, 200, 120)

    content = line(10, 90, 190, 90) + path("M185 85 L190 90 L185 95") + \
              line(10, 90, 10, 10) + path("M5 15 L10 10 L15 15") + \
              text(190, 105, "f") + text(5, 20, "A") + text(5, 95, "0")
    save_svg(os.path.join(out_dir, "coord_f_A.svg"), content, 200, 120)

    # 2.2 Periodic AC Waveforms
    sine_path = "M 10 50 Q 30 10 50 50 T 90 50"
    content = line(0, 50, 100, 50, stroke="#ccc", width="1") + path(sine_path)
    save_svg(os.path.join(out_dir, "wave_sine.svg"), content, 100, 100)

    sine_path_2 = "M 10 50 Q 20 10 30 50 T 50 50 Q 60 10 70 50 T 90 50"
    content = line(0, 50, 100, 50, stroke="#ccc", width="1") + path(sine_path_2)
    save_svg(os.path.join(out_dir, "wave_sine_2p.svg"), content, 100, 100)

    content = line(0, 50, 100, 50, stroke="#ccc", width="1") + \
              path("M 10 50 L 10 20 L 50 20 L 50 80 L 90 80 L 90 50")
    save_svg(os.path.join(out_dir, "wave_square.svg"), content, 100, 100)

    content = line(0, 50, 100, 50, stroke="#ccc", width="1") + \
              path("M 10 50 L 30 20 L 70 80 L 90 50")
    save_svg(os.path.join(out_dir, "wave_triangle.svg"), content, 100, 100)

    content = line(0, 50, 100, 50, stroke="#ccc", width="1") + \
              path("M 10 80 L 50 20 L 50 80 L 90 20 L 90 80")
    save_svg(os.path.join(out_dir, "wave_sawtooth.svg"), content, 100, 100)

    # 2.3 Phasor Plane
    content = circle(50, 50, 40) + line(10, 50, 90, 50) + line(50, 10, 50, 90) + \
              text(95, 53, "0°", "10") + text(50, 8, "90°", "10") + \
              text(5, 53, "180°", "10") + text(50, 98, "270°", "10")
    save_svg(os.path.join(out_dir, "phasor_plane.svg"), content, 100, 100)

    # 2.4 Bode Plot Template (Simplified)
    content = rect(10, 10, 180, 80)
    for i in range(1, 10):
        x1 = 10 + 90 * math.log10(i)
        x2 = 100 + 90 * math.log10(i)
        if x1 <= 190:
            content += line(x1, 10, x1, 90, stroke="#eee", width="1")
        if x2 <= 190:
            content += line(x2, 10, x2, 90, stroke="#eee", width="1")
    content += line(10, 50, 190, 50, stroke="#ccc", width="1")
    save_svg(os.path.join(out_dir, "bode_grid_2dec.svg"), content, 200, 100)

    content = rect(10, 10, 280, 80)
    for i in range(1, 10):
        x1 = 10 + 90 * math.log10(i)
        x2 = 100 + 90 * math.log10(i)
        x3 = 190 + 90 * math.log10(i)
        if x1 <= 280: content += line(x1, 10, x1, 90, stroke="#eee", width="1")
        if x2 <= 280: content += line(x2, 10, x2, 90, stroke="#eee", width="1")
        if x3 <= 280: content += line(x3, 10, x3, 90, stroke="#eee", width="1")
    content += line(10, 50, 280, 50, stroke="#ccc", width="1")
    save_svg(os.path.join(out_dir, "bode_grid_3dec.svg"), content, 300, 100)

def gen_passives(base_dir):
    out_dir = os.path.join(base_dir, "plugins", "electronics-suite", "assets", "passives")

    # Resistors
    content = path("M 0 50 L 20 50 L 25 30 L 35 70 L 45 30 L 55 70 L 65 30 L 75 70 L 80 50 L 100 50")
    save_svg(os.path.join(out_dir, "resistor_us.svg"), content, 100, 100)

    content = line(0, 50, 20, 50) + rect(20, 40, 60, 20) + line(80, 50, 100, 50)
    save_svg(os.path.join(out_dir, "resistor_eu.svg"), content, 100, 100)

    content = path("M 0 50 L 20 50 L 25 30 L 35 70 L 45 30 L 55 70 L 65 30 L 75 70 L 80 50 L 100 50") + \
              line(50, 20, 50, 40) + path("M 45 35 L 50 40 L 55 35")
    save_svg(os.path.join(out_dir, "potentiometer.svg"), content, 100, 100)

    # Capacitors
    content = line(0, 50, 40, 50) + line(40, 20, 40, 80) + line(60, 20, 60, 80) + line(60, 50, 100, 50)
    save_svg(os.path.join(out_dir, "capacitor_np.svg"), content, 100, 100)

    content = line(0, 50, 40, 50) + line(40, 20, 40, 80) + \
              path("M 70 20 Q 50 50 70 80") + line(60, 50, 100, 50) + \
              text(30, 20, "+", "14")
    save_svg(os.path.join(out_dir, "capacitor_pol.svg"), content, 100, 100)

    # Inductors
    content = line(0, 50, 20, 50) + \
              path("M 20 50 A 10 10 0 0 1 40 50") + \
              path("M 40 50 A 10 10 0 0 1 60 50") + \
              path("M 60 50 A 10 10 0 0 1 80 50") + \
              line(80, 50, 100, 50)
    save_svg(os.path.join(out_dir, "inductor.svg"), content, 100, 100)

    # Sources
    content = line(50, 0, 50, 30) + line(30, 30, 70, 30) + line(40, 40, 60, 40) + \
              line(30, 50, 70, 50) + line(40, 60, 60, 60) + line(50, 60, 50, 100) + \
              text(65, 20, "+", "14")
    save_svg(os.path.join(out_dir, "source_dc_batt.svg"), content, 100, 100)

    content = line(50, 0, 50, 20) + circle(50, 50, 30) + line(50, 80, 50, 100) + \
              text(65, 20, "+", "14") + text(65, 90, "-", "14")
    save_svg(os.path.join(out_dir, "source_dc.svg"), content, 100, 100)

    content = line(50, 0, 50, 20) + circle(50, 50, 30) + line(50, 80, 50, 100) + \
              path("M 35 50 Q 42.5 35 50 50 T 65 50")
    save_svg(os.path.join(out_dir, "source_ac.svg"), content, 100, 100)

    content = line(50, 0, 50, 20) + circle(50, 50, 30) + line(50, 80, 50, 100) + \
              line(50, 35, 50, 65) + path("M 45 45 L 50 35 L 55 45")
    save_svg(os.path.join(out_dir, "source_current.svg"), content, 100, 100)

    # Grounds
    content = line(50, 0, 50, 50) + line(20, 50, 80, 50) + line(30, 60, 70, 60) + line(40, 70, 60, 70)
    save_svg(os.path.join(out_dir, "gnd_earth.svg"), content, 100, 100)

    content = line(50, 0, 50, 50) + line(20, 50, 80, 50) + line(30, 30, 50, 50) + line(70, 30, 50, 50)
    save_svg(os.path.join(out_dir, "gnd_chassis.svg"), content, 100, 100)

    content = line(50, 0, 50, 50) + path("M 30 50 L 70 50 L 50 80 Z")
    save_svg(os.path.join(out_dir, "gnd_signal.svg"), content, 100, 100)

    # Switches
    content = line(0, 50, 30, 50) + circle(30, 50, 2) + line(30, 50, 65, 35) + circle(70, 50, 2) + line(70, 50, 100, 50)
    save_svg(os.path.join(out_dir, "switch_spst.svg"), content, 100, 100)

    content = line(0, 50, 30, 50) + circle(30, 50, 2) + line(30, 50, 65, 35) + \
              circle(70, 30, 2) + line(70, 30, 100, 30) + \
              circle(70, 70, 2) + line(70, 70, 100, 70)
    save_svg(os.path.join(out_dir, "switch_spdt.svg"), content, 100, 100)

def gen_semiconductors(base_dir):
    out_dir = os.path.join(base_dir, "plugins", "electronics-suite", "assets", "semiconductors")

    # Diodes
    content = line(0, 50, 40, 50) + path("M 40 30 L 60 50 L 40 70 Z") + line(60, 30, 60, 70) + line(60, 50, 100, 50)
    save_svg(os.path.join(out_dir, "diode.svg"), content, 100, 100)

    content = line(0, 50, 40, 50) + path("M 40 30 L 60 50 L 40 70 Z") + line(60, 30, 60, 70) + line(60, 50, 100, 50) + \
              path("M 60 30 L 65 30 L 65 35 M 60 70 L 55 70 L 55 65")
    save_svg(os.path.join(out_dir, "diode_zener.svg"), content, 100, 100)

    content = line(0, 50, 40, 50) + path("M 40 30 L 60 50 L 40 70 Z") + line(60, 30, 60, 70) + line(60, 50, 100, 50) + \
              path("M 60 30 L 65 30 L 65 40 M 60 70 L 55 70 L 55 60")
    save_svg(os.path.join(out_dir, "diode_schottky.svg"), content, 100, 100)

    content = line(0, 50, 40, 50) + path("M 40 30 L 60 50 L 40 70 Z") + line(60, 30, 60, 70) + line(60, 50, 100, 50) + \
              line(70, 20, 85, 5) + path("M 80 5 L 85 5 L 85 10") + \
              line(80, 30, 95, 15) + path("M 90 15 L 95 15 L 95 20")
    save_svg(os.path.join(out_dir, "diode_led.svg"), content, 100, 100)

    # BJTs
    content = line(10, 50, 35, 50) + line(35, 30, 35, 70) + \
              line(35, 40, 65, 10) + line(65, 10, 65, 0) + \
              line(35, 60, 65, 90) + line(65, 90, 65, 100) + \
              path("M 50 75 L 65 90 L 60 70 Z", fill="black")
    save_svg(os.path.join(out_dir, "bjt_npn.svg"), content, 100, 100)

    content = line(10, 50, 35, 50) + line(35, 30, 35, 70) + \
              line(35, 40, 65, 10) + line(65, 10, 65, 0) + \
              line(35, 60, 65, 90) + line(65, 90, 65, 100) + \
              path("M 60 70 L 35 40 L 50 45 Z", fill="black")
    save_svg(os.path.join(out_dir, "bjt_pnp.svg"), content, 100, 100)

    # MOSFETs
    content = line(10, 50, 30, 50) + line(30, 20, 30, 80) + \
              line(40, 25, 40, 45) + line(40, 48, 40, 68) + line(40, 70, 40, 90) + \
              line(40, 35, 70, 35) + line(70, 35, 70, 0) + \
              line(40, 75, 70, 75) + line(70, 75, 70, 100) + \
              line(40, 58, 70, 58) + line(70, 58, 70, 75) + \
              path("M 55 58 L 45 53 L 45 63 Z", fill="black")
    save_svg(os.path.join(out_dir, "mosfet_n.svg"), content, 100, 100)

    content = line(10, 50, 30, 50) + line(30, 20, 30, 80) + \
              line(40, 25, 40, 45) + line(40, 48, 40, 68) + line(40, 70, 40, 90) + \
              line(40, 35, 70, 35) + line(70, 35, 70, 0) + \
              line(40, 75, 70, 75) + line(70, 75, 70, 100) + \
              line(40, 58, 70, 58) + line(70, 58, 70, 75) + \
              path("M 45 58 L 55 53 L 55 63 Z", fill="black")
    save_svg(os.path.join(out_dir, "mosfet_p.svg"), content, 100, 100)

    # Op-Amp
    content = path("M 20 20 L 80 50 L 20 80 Z") + \
              line(0, 35, 20, 35) + text(25, 40, "-") + \
              line(0, 65, 20, 65) + text(25, 70, "+") + \
              line(80, 50, 100, 50) + \
              line(50, 35, 50, 0) + text(50, -5, "V+") + \
              line(50, 65, 50, 100) + text(50, 115, "V-")
    save_svg(os.path.join(out_dir, "opamp.svg"), content, 100, 120, "-20 -20 140 160")

def gen_digital(base_dir):
    out_dir = os.path.join(base_dir, "plugins", "electronics-suite", "assets", "digital")

    # Logic Gates
    content = line(0, 30, 20, 30) + line(0, 70, 20, 70) + \
              path("M 20 10 L 40 10 A 40 40 0 0 1 40 90 L 20 90 Z") + line(80, 50, 100, 50)
    save_svg(os.path.join(out_dir, "gate_and.svg"), content, 100, 100)

    content = line(0, 30, 20, 30) + line(0, 70, 20, 70) + \
              path("M 10 10 Q 30 50 10 90 Q 60 90 90 50 Q 60 10 10 10") + line(90, 50, 100, 50)
    save_svg(os.path.join(out_dir, "gate_or.svg"), content, 100, 100)

    content = line(0, 50, 20, 50) + path("M 20 20 L 70 50 L 20 80 Z") + \
              circle(75, 50, 5) + line(80, 50, 100, 50)
    save_svg(os.path.join(out_dir, "gate_not.svg"), content, 100, 100)

    content = line(0, 30, 20, 30) + line(0, 70, 20, 70) + \
              path("M 20 10 L 40 10 A 40 40 0 0 1 40 90 L 20 90 Z") + \
              circle(85, 50, 5) + line(90, 50, 100, 50)
    save_svg(os.path.join(out_dir, "gate_nand.svg"), content, 100, 100)

    content = line(0, 30, 20, 30) + line(0, 70, 20, 70) + \
              path("M 10 10 Q 30 50 10 90 Q 60 90 90 50 Q 60 10 10 10") + \
              circle(95, 50, 5) + line(100, 50, 110, 50)
    save_svg(os.path.join(out_dir, "gate_nor.svg"), content, 110, 100)

    content = line(0, 30, 20, 30) + line(0, 70, 20, 70) + \
              path("M 10 10 Q 30 50 10 90", stroke="black", width="2", fill="none") + \
              path("M 15 10 Q 35 50 15 90 Q 65 90 95 50 Q 65 10 15 10") + \
              line(95, 50, 100, 50)
    save_svg(os.path.join(out_dir, "gate_xor.svg"), content, 100, 100)

    content = line(0, 30, 20, 30) + line(0, 70, 20, 70) + \
              path("M 10 10 Q 30 50 10 90", stroke="black", width="2", fill="none") + \
              path("M 15 10 Q 35 50 15 90 Q 65 90 95 50 Q 65 10 15 10") + \
              circle(100, 50, 5) + line(105, 50, 115, 50)
    save_svg(os.path.join(out_dir, "gate_xnor.svg"), content, 115, 100)

    # Flip-Flops
    content = rect(20, 10, 60, 80) + \
              line(0, 30, 20, 30) + text(28, 35, "D") + \
              line(0, 70, 20, 70) + path("M 20 65 L 30 70 L 20 75") + \
              line(80, 30, 100, 30) + text(70, 35, "Q") + \
              line(80, 70, 100, 70) + text(70, 75, "Q'") + line(70, 60, 75, 60)
    save_svg(os.path.join(out_dir, "ff_d.svg"), content, 100, 100)

    content = rect(20, 10, 60, 80) + \
              line(0, 30, 20, 30) + text(28, 35, "J") + \
              line(0, 70, 20, 70) + text(28, 75, "K") + \
              line(0, 50, 20, 50) + path("M 20 45 L 30 50 L 20 55") + \
              line(80, 30, 100, 30) + text(70, 35, "Q") + \
              line(80, 70, 100, 70) + text(70, 75, "Q'") + line(70, 60, 75, 60)
    save_svg(os.path.join(out_dir, "ff_jk.svg"), content, 100, 100)

    # K-Maps
    content = rect(30, 30, 60, 60) + line(60, 30, 60, 90) + line(30, 60, 90, 60) + \
              line(10, 10, 30, 30) + text(25, 15, "A") + text(15, 25, "B") + \
              text(45, 25, "0") + text(75, 25, "1") + \
              text(20, 45, "0") + text(20, 75, "1")
    save_svg(os.path.join(out_dir, "kmap_2x2.svg"), content, 100, 100)

    content = rect(30, 30, 120, 60) + \
              line(60, 30, 60, 90) + line(90, 30, 90, 90) + line(120, 30, 120, 90) + \
              line(30, 60, 150, 60) + \
              line(10, 10, 30, 30) + text(25, 15, "AB") + text(15, 25, "C") + \
              text(45, 25, "00") + text(75, 25, "01") + text(105, 25, "11") + text(135, 25, "10") + \
              text(20, 45, "0") + text(20, 75, "1")
    save_svg(os.path.join(out_dir, "kmap_2x4.svg"), content, 160, 100)

    content = rect(30, 30, 120, 120) + \
              line(60, 30, 60, 150) + line(90, 30, 90, 150) + line(120, 30, 120, 150) + \
              line(30, 60, 150, 60) + line(30, 90, 150, 90) + line(30, 120, 150, 120) + \
              line(10, 10, 30, 30) + text(25, 15, "AB") + text(15, 25, "CD") + \
              text(45, 25, "00") + text(75, 25, "01") + text(105, 25, "11") + text(135, 25, "10") + \
              text(15, 45, "00") + text(15, 75, "01") + text(15, 105, "11") + text(15, 135, "10")
    save_svg(os.path.join(out_dir, "kmap_4x4.svg"), content, 160, 160)

    # Timing Diagram
    content = ""
    for i in range(4):
        y = 20 + i * 20
        content += line(0, y, 100, y, stroke="#ccc", width="1")
    for i in range(5):
        x = 20 + i * 20
        content += line(x, 0, x, 100, stroke="#eee", width="1")
    save_svg(os.path.join(out_dir, "timing_grid.svg"), content, 100, 100)

    # Truth Table Grid
    content = rect(10, 10, 80, 100) + line(10, 30, 90, 30) + line(50, 10, 50, 110) + \
              text(30, 25, "In") + text(70, 25, "Out")
    save_svg(os.path.join(out_dir, "truth_table.svg"), content, 100, 120)

    # Register 8-bit
    content = rect(10, 10, 160, 30)
    for i in range(1, 8): content += line(10 + i * 20, 10, 10 + i * 20, 40)
    for i in range(8): content += text(20 + i * 20, 28, f"D{7-i}", "10")
    save_svg(os.path.join(out_dir, "register_8bit.svg"), content, 180, 50)

    # Register 16-bit
    content = rect(10, 10, 320, 30)
    for i in range(1, 16): content += line(10 + i * 20, 10, 10 + i * 20, 40)
    for i in range(16): content += text(20 + i * 20, 28, f"D{15-i}", "10")
    save_svg(os.path.join(out_dir, "register_16bit.svg"), content, 340, 50)

    # Flowcharts
    content = rect(10, 10, 80, 40, rx="20", ry="20", fill="none", stroke="black", width="2") + text(50, 35, "Start/End")
    save_svg(os.path.join(out_dir, "flow_start.svg"), content, 100, 60)

    content = rect(10, 10, 80, 40) + text(50, 35, "Process")
    save_svg(os.path.join(out_dir, "flow_process.svg"), content, 100, 60)

    content = path("M 50 10 L 90 30 L 50 50 L 10 30 Z") + text(50, 35, "Decision")
    save_svg(os.path.join(out_dir, "flow_decision.svg"), content, 100, 60)

    content = path("M 20 10 L 90 10 L 80 50 L 10 50 Z") + text(50, 35, "I/O")
    save_svg(os.path.join(out_dir, "flow_io.svg"), content, 100, 60)

def gen_blocks(base_dir):
    out_dir = os.path.join(base_dir, "plugins", "electronics-suite", "assets", "blocks")

    # Mixer
    content = circle(50, 50, 20) + line(35, 35, 65, 65) + line(35, 65, 65, 35) + \
              line(0, 50, 30, 50) + line(70, 50, 100, 50) + line(50, 0, 50, 30)
    save_svg(os.path.join(out_dir, "mixer.svg"), content, 100, 100)

    # Amplifier
    content = path("M 20 20 L 80 50 L 20 80 Z") + line(0, 50, 20, 50) + line(80, 50, 100, 50)
    save_svg(os.path.join(out_dir, "amplifier.svg"), content, 100, 100)

    # LPF Box
    content = rect(20, 20, 60, 60) + line(0, 50, 20, 50) + line(80, 50, 100, 50) + \
              text(50, 55, "LPF")
    save_svg(os.path.join(out_dir, "filter_lpf.svg"), content, 100, 100)

    # HPF Box
    content = rect(20, 20, 60, 60) + line(0, 50, 20, 50) + line(80, 50, 100, 50) + \
              text(50, 55, "HPF")
    save_svg(os.path.join(out_dir, "filter_hpf.svg"), content, 100, 100)

    # Oscillator Box
    content = rect(20, 20, 60, 60) + line(80, 50, 100, 50) + \
              path("M 30 50 Q 40 30 50 50 T 70 50", stroke="black", width="1")
    save_svg(os.path.join(out_dir, "oscillator.svg"), content, 100, 100)

    # Antenna
    content = line(50, 50, 50, 100) + path("M 50 50 L 30 20 L 70 20 Z")
    save_svg(os.path.join(out_dir, "antenna.svg"), content, 100, 100)

def gen_lua_plugin(base_dir):
    plugin_dir = os.path.join(base_dir, "plugins", "electronics-suite")

    plugin_ini = """[about]
author=Electronics Suite Generator
description=Electronics Engineering Toolkit for Xournal++
version=<xournalpp>
[default]
enabled=true
[plugin]
mainfile=main.lua
"""
    with open(os.path.join(plugin_dir, "plugin.ini"), "w") as f: f.write(plugin_ini)

    main_lua = "-- Xournal++ Electronics Suite Plugin\nfunction initUi()\n  app.registerUi({menu=\"Electronics\", callback=\"dummy\", parentPath=\"\"})\n"
    categories = {
        "Analog & Passives": {
            "Resistor (US)": "passives/resistor_us.svg", "Resistor (EU)": "passives/resistor_eu.svg", "Potentiometer": "passives/potentiometer.svg",
            "Capacitor (Non-Polarized)": "passives/capacitor_np.svg", "Capacitor (Polarized)": "passives/capacitor_pol.svg",
            "Inductor": "passives/inductor.svg",
            "DC Source (Battery)": "passives/source_dc_batt.svg", "DC Source (Circle)": "passives/source_dc.svg", "AC Source": "passives/source_ac.svg", "Current Source": "passives/source_current.svg",
            "Earth Ground": "passives/gnd_earth.svg", "Chassis Ground": "passives/gnd_chassis.svg", "Signal Ground": "passives/gnd_signal.svg",
            "SPST Switch": "passives/switch_spst.svg", "SPDT Switch": "passives/switch_spdt.svg"
        },
        "Semiconductors & Op-Amps": {
            "Diode": "semiconductors/diode.svg", "Zener Diode": "semiconductors/diode_zener.svg", "Schottky Diode": "semiconductors/diode_schottky.svg", "LED": "semiconductors/diode_led.svg",
            "BJT (NPN)": "semiconductors/bjt_npn.svg", "BJT (PNP)": "semiconductors/bjt_pnp.svg",
            "MOSFET (N-Channel)": "semiconductors/mosfet_n.svg", "MOSFET (P-Channel)": "semiconductors/mosfet_p.svg",
            "Op-Amp": "semiconductors/opamp.svg"
        },
        "Digital Logic & Gates": {
            "AND Gate": "digital/gate_and.svg", "OR Gate": "digital/gate_or.svg", "NOT Gate": "digital/gate_not.svg",
            "NAND Gate": "digital/gate_nand.svg", "NOR Gate": "digital/gate_nor.svg", "XOR Gate": "digital/gate_xor.svg", "XNOR Gate": "digital/gate_xnor.svg",
            "D Flip-Flop": "digital/ff_d.svg", "JK Flip-Flop": "digital/ff_jk.svg",
            "Mixer": "blocks/mixer.svg", "Amplifier": "blocks/amplifier.svg",
            "Low-Pass Filter": "blocks/filter_lpf.svg", "High-Pass Filter": "blocks/filter_hpf.svg", "Oscillator": "blocks/oscillator.svg", "Antenna": "blocks/antenna.svg"
        },
        "Waveforms & Graphs": {
            "Sine Wave": "waveforms/wave_sine.svg", "Sine Wave (2 Periods)": "waveforms/wave_sine_2p.svg",
            "Square Wave": "waveforms/wave_square.svg", "Triangle Wave": "waveforms/wave_triangle.svg", "Sawtooth Wave": "waveforms/wave_sawtooth.svg",
            "t vs V Axis": "waveforms/coord_t_V.svg", "f vs A Axis": "waveforms/coord_f_A.svg", "Phasor Plane": "waveforms/phasor_plane.svg"
        },
        "Analysis Grids (K-Maps, Timing, Bode)": {
            "Bode Plot (2 Decades)": "waveforms/bode_grid_2dec.svg", "Bode Plot (3 Decades)": "waveforms/bode_grid_3dec.svg",
            "K-Map 2x2": "digital/kmap_2x2.svg", "K-Map 2x4": "digital/kmap_2x4.svg", "K-Map 4x4": "digital/kmap_4x4.svg",
            "Timing Diagram": "digital/timing_grid.svg", "Truth Table": "digital/truth_table.svg",
            "Register (8-bit)": "digital/register_8bit.svg", "Register (16-bit)": "digital/register_16bit.svg",
            "Flowchart Start/End": "digital/flow_start.svg", "Flowchart Process": "digital/flow_process.svg", "Flowchart Decision": "digital/flow_decision.svg", "Flowchart I/O": "digital/flow_io.svg"
        }
    }

    callback_idx = 1
    callbacks = ""
    for cat_name, items in categories.items():
        for item_name, rel_path in items.items():
            func_name = f"insert_asset_{callback_idx}"
            main_lua += f'  app.registerUi({{menu="{item_name}", callback="{func_name}", parentPath="Electronics/{cat_name}"}})\n'
            callbacks += f"function {func_name}()\n  app.addImages({{images = {{{{path = app.getPluginDir() .. \"/assets/{rel_path}\", x = 100, y = 100}}}}}})\n  app.uiAction({{action=\"ACTION_UPDATE_PAGE_BACKGROUND\"}})\nend\n"
            callback_idx += 1

    main_lua += "end\nfunction dummy()\nend\n" + callbacks
    with open(os.path.join(plugin_dir, "main.lua"), "w") as f: f.write(main_lua)

def gen_configs(base_dir):
    config_dir = os.path.join(base_dir, "config")
    toolbar_ini = """[Electronics Student Portrait]
name=Electronics Student Portrait
toolbarTop1 = SAVE,NEW,OPEN,SEPARATOR,SAVEPDF,PRINT,SEPARATOR,CUT,COPY,PASTE,SEPARATOR,UNDO,REDO,SEPARATOR,GOTO_FIRST,GOTO_BACK,GOTO_NEXT_ANNOTATED_PAGE,GOTO_NEXT,GOTO_LAST,INSERT_NEW_PAGE,DELETE_CURRENT_PAGE,SEPARATOR,FULLSCREEN
toolbarTop2 = PEN,ERASER,HIGHLIGHTER,IMAGE,TEXT,MATH_TEX,DRAW,SEPARATOR,ROTATION_SNAPPING,GRID_SNAPPING,SEPARATOR,SELECT,VERTICAL_SPACE,HAND,SETSQUARE,COMPASS,SEPARATOR,DEFAULT_TOOL,SEPARATOR,FINE,MEDIUM,THICK,SEPARATOR,TOOL_FILL,SEPARATOR,COLOR(0),COLOR(1),COLOR(2),COLOR(3),COLOR(4),COLOR(5),COLOR_SELECT
toolbarBottom1 = PAGE_SPIN,SEPARATOR,LAYER,SPACER,PAIRED_PAGES,ZOOM_100,ZOOM_FIT,ZOOM_OUT,ZOOM_SLIDER,ZOOM_IN
[Electronics Student Landscape]
name=Electronics Student Landscape
toolbarTop1 = SAVE,NEW,OPEN,SEPARATOR,SAVEPDF,PRINT,SEPARATOR,CUT,COPY,PASTE,SEPARATOR,UNDO,REDO,SEPARATOR,PEN,ERASER,HIGHLIGHTER,IMAGE,TEXT,MATH_TEX,DRAW,SEPARATOR,SELECT,VERTICAL_SPACE,HAND,SETSQUARE,COMPASS,SEPARATOR,FINE,MEDIUM,THICK,SEPARATOR,COLOR(0),COLOR(1),COLOR(2),COLOR(3),COLOR(4),COLOR(5),COLOR_SELECT
toolbarBottom1 = PAGE_SPIN,SEPARATOR,LAYER,GOTO_FIRST,GOTO_NEXT_ANNOTATED_PAGE,GOTO_LAST,INSERT_NEW_PAGE,DELETE_CURRENT_PAGE,SPACER,PAIRED_PAGES,ZOOM_100,ZOOM_FIT,ZOOM_OUT,ZOOM_SLIDER,ZOOM_IN,SEPARATOR,FULLSCREEN
"""
    with open(os.path.join(config_dir, "toolbar.ini"), "w") as f: f.write(toolbar_ini)

    palette_gpl = """GIMP Palette
Name: Electronics Engineering Palette
Columns: 6
#
  0   0   0 Circuit Black
211  47  47 VCC / Power Red
 25 118 212 Ground / Return Blue
 56 142  60 Signal Green
251 192  45 Component Highlight Yellow
123  31 162 Annotation Purple
"""
    with open(os.path.join(config_dir, "palette.gpl"), "w") as f: f.write(palette_gpl)

def main():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    setup_dirs(base_dir)
    gen_waveforms(base_dir)
    gen_passives(base_dir)
    gen_semiconductors(base_dir)
    gen_digital(base_dir)
    gen_blocks(base_dir)
    gen_lua_plugin(base_dir)
    gen_configs(base_dir)
    print("Assets fully generated procedurally.")

if __name__ == "__main__":
    main()
