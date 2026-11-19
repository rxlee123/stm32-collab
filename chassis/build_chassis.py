"""Printable two-deck chassis for the AIROST delivery robot.

Units: mm.  Axes (assembled robot): X = left/right, Y = forward (+Y is the front),
Z = up, floor at Z = 0.  Every part is modelled in its assembled position, then
exported twice: as assembled (for the viewer) and laid flat for printing.

Change the numbers in the PARAMETERS block and re-run:  python3 build_chassis.py
"""
import json, os
import numpy as np
import trimesh
from manifold3d import Manifold

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
os.makedirs(OUT, exist_ok=True)

# ---------------- PARAMETERS ----------------
DECK_W = 136          # lower deck width (wheels sit just outside it)
TOP_W = 156           # top deck width (fits the 150 mm mainboard; sits above the wheels)
DECK_T = 3            # deck thickness
LOW_Y = (-80, 110)    # lower deck front/back
TOP_Y = (-70, 55)     # top deck front/back
LOW_Z = 44.5          # underside of lower deck above floor (65 mm wheels, TT motor under deck)
GAP = 37              # space between decks (standoff length)
STANDOFF_XY = [(-56, -64), (56, -64), (-62, 49), (62, 49)]
M3 = 3.4              # clearance hole for M3 screws

AXLE_Y, AXLE_Z = -45, 32.5   # wheel axle position (65 mm wheel radius)
MOTOR_X = 55                 # centre of each TT gearbox, left/right
GEARBOX_W = 18.8             # TT gearbox thickness (X)

CASTER_Y = -12
SENSOR_BOARD = dict(cx=0, cy=68, w=90, d=73, top_z=19, inset=3)   # your IR sensor board
COLOUR_Y = 18
COLOUR_TOP_Z = 15

BALL_D = 40
CAGE_IN_W = 46        # inside width of the claw cage
CAGE_BACK_Y = 107     # back wall starts here
CAGE_DEPTH = 45       # inside depth
CAGE_WALL = 3
CAGE_BOTTOM_Z = 10    # clears the ring under the ball
CAGE_PLATE_Z = 33     # underside of the cage top plate

SG90 = dict(L=22.8, W=12.2, tab_hole_pitch=27.5, shaft_from_end=5.9)
MAINBOARD = dict(w=150, d=90, cx=0, cy=-9, inset=3)
SEG = 48
# ---------------------------------------------

def box(x0, x1, y0, y1, z0, z1):
    return Manifold.cube([x1 - x0, y1 - y0, z1 - z0]).translate([x0, y0, z0])

def cyl_z(x, y, z0, z1, r, seg=SEG):
    return Manifold.cylinder(z1 - z0, r, r, seg).translate([x, y, z0])

def cyl_x(x0, x1, y, z, r, seg=SEG):
    return Manifold.cylinder(x1 - x0, r, r, seg).rotate([0, 90, 0]).translate([x0, y, z])

def slot_z(x0, y0, x1, y1, z0, z1, r):
    return Manifold.batch_hull([cyl_z(x0, y0, z0, z1, r), cyl_z(x1, y1, z0, z1, r)])

def slot_x(xa, xb, y0, z0, y1, z1, r):
    return Manifold.batch_hull([cyl_x(xa, xb, y0, z0, r), cyl_x(xa, xb, y1, z1, r)])

def union(parts):
    out = parts[0]
    for p in parts[1:]:
        out = out + p
    return out

def rounded_plate(x0, x1, y0, y1, z0, z1, r=6):
    pts = [cyl_z(x, y, z0, z1, r) for x in (x0 + r, x1 - r) for y in (y0 + r, y1 - r)]
    return Manifold.batch_hull(pts)

lz0, lz1 = LOW_Z, LOW_Z + DECK_T
tz0 = lz1 + GAP
tz1 = tz0 + DECK_T
H = DECK_W / 2
holes_info = {}

# ======================= LOWER DECK =======================
low = rounded_plate(-H, H, LOW_Y[0], LOW_Y[1], lz0, lz1)
cut = []
cut += [cyl_z(x, y, lz0 - 1, lz1 + 1, M3 / 2) for x, y in STANDOFF_XY]

# TT motor brackets hang under the deck, on the inner side of each gearbox.
for s in (-1, 1):
    inner = s * (MOTOR_X - GEARBOX_W / 2)            # gearbox inner face
    bx0, bx1 = sorted([inner, inner - s * 4])         # 4 mm plate
    plate = box(bx0, bx1, AXLE_Y - 16, AXLE_Y + 25, AXLE_Z - 12, lz0 + 0.01)
    rib = box(min(bx0, bx1 - s * 0), max(bx0, bx1), AXLE_Y - 16, AXLE_Y + 25, lz0 - 4, lz0 + 0.01)
    gus = Manifold.batch_hull([box(bx0, bx1, AXLE_Y + 21, AXLE_Y + 25, AXLE_Z - 12, lz0),
                               box(min(bx0, bx1) - (8 if s > 0 else 0), max(bx0, bx1) + (8 if s < 0 else 0),
                                   AXLE_Y + 21, AXLE_Y + 25, lz0 - 0.5, lz0)])
    low = low + plate + rib + gus
    # slot along Y at axle height: fits TT mounting holes wherever they fall
    cut.append(slot_x(bx0 - 1, bx1 + 1, AXLE_Y - 8, AXLE_Z, AXLE_Y + 22, AXLE_Z, M3 / 2))
    cut.append(cyl_x(bx0 - 1, bx1 + 1, AXLE_Y, AXLE_Z, 4.5))  # clearance for inner shaft stub
    # cable-tie slots either side of the gearbox, as a backup fixing
    for yy in (AXLE_Y + 2, AXLE_Y + 24):
        for xx in (s * (MOTOR_X - GEARBOX_W / 2 - 7), s * (MOTOR_X + GEARBOX_W / 2 + 3)):
            cut.append(box(xx - 1.5, xx + 1.5, yy - 2.5, yy + 2.5, lz0 - 1, lz1 + 1))
    # motor wire hole
    cut.append(slot_z(s * 32, -30, s * 32, -18, lz0 - 1, lz1 + 1, 4))

# ball caster: two slots, fit hole spacings 16-44 mm
for s in (-1, 1):
    cut.append(slot_z(s * 8, CASTER_Y, s * 22, CASTER_Y, lz0 - 1, lz1 + 1, M3 / 2))

# IR sensor board: 4 corner holes
sb = SENSOR_BOARD
sb_holes = [(sb["cx"] + sx * (sb["w"] / 2 - sb["inset"]), sb["cy"] + sy * (sb["d"] / 2 - sb["inset"]))
            for sx in (-1, 1) for sy in (-1, 1)]
cut += [cyl_z(x, y, lz0 - 1, lz1 + 1, M3 / 2) for x, y in sb_holes]
# colour sensor: two slots (fits boards with 12-22 mm hole spacing)
for s in (-1, 1):
    cut.append(slot_z(s * 6, COLOUR_Y, s * 11, COLOUR_Y, lz0 - 1, lz1 + 1, 1.6))
# claw cage screws
CAGE_SCREWS = [(-12, 95), (12, 95)]
cut += [cyl_z(x, y, lz0 - 1, lz1 + 1, M3 / 2) for x, y in CAGE_SCREWS]
# front wire hole (sensors, colour sensor, servo -> top deck)
cut.append(slot_z(-6, 52, 6, 52, lz0 - 1, lz1 + 1, 7))
# battery strap slots (cable ties / velcro) for the two battery packs
for (yc, half) in ((-56, 22), (4, 34)):
    for s in (-1, 1):
        cut.append(box(s * 40 - 1.5, s * 40 + 1.5, yc - 6, yc + 6, lz0 - 1, lz1 + 1))
# wheel wells: notches so the wheels don't rub the deck edge
WHEEL_CLEAR = 36
for s in (-1, 1):
    cut.append(box(min(s*63.5, s*80), max(s*63.5, s*80), AXLE_Y - WHEEL_CLEAR, AXLE_Y + WHEEL_CLEAR, lz0 - 1, lz1 + 1))
lower_deck = low - union(cut)

# ======================= TOP DECK =======================
top = rounded_plate(-TOP_W / 2, TOP_W / 2, TOP_Y[0], TOP_Y[1], tz0, tz1)
cut = [cyl_z(x, y, tz0 - 1, tz1 + 1, M3 / 2) for x, y in STANDOFF_XY]
mb = MAINBOARD
mb_holes = [(mb["cx"] + sx * (mb["w"] / 2 - mb["inset"]), mb["cy"] + sy * (mb["d"] / 2 - mb["inset"]))
            for sx in (-1, 1) for sy in (-1, 1)]
cut += [cyl_z(x, y, tz0 - 1, tz1 + 1, M3 / 2) for x, y in mb_holes]
cut.append(slot_z(-8, 43, 8, 43, tz0 - 1, tz1 + 1, 7))      # front wire hole
cut.append(slot_z(-14, -62, 14, -62, tz0 - 1, tz1 + 1, 4))  # rear wire slot (motors, batteries)
# lightening windows under the mainboard (keeps the deck light and lets you reach solder joints)
for sx in (-1, 1):
    cut.append(rounded_plate(sx * 8 if sx > 0 else -52, 52 if sx > 0 else -8, -40, 20, tz0 - 1, tz1 + 1, 5))
top_deck = top - union(cut)

# ======================= CLAW CAGE =======================
iw = CAGE_IN_W / 2
front = CAGE_BACK_Y + CAGE_WALL + CAGE_DEPTH
pz0, pz1 = CAGE_PLATE_Z, CAGE_PLATE_Z + 3
walls = [
    box(-iw - CAGE_WALL, -iw, CAGE_BACK_Y, front, CAGE_BOTTOM_Z, pz1),
    box(iw, iw + CAGE_WALL, CAGE_BACK_Y, front, CAGE_BOTTOM_Z, pz1),
    box(-iw - CAGE_WALL, iw + CAGE_WALL, CAGE_BACK_Y, CAGE_BACK_Y + CAGE_WALL, CAGE_BOTTOM_Z, pz1),
]
plate = box(-iw - CAGE_WALL, iw + CAGE_WALL, 85, front, pz0, pz1)
# servo pad at the front-left corner, servo hangs upside down through it
sv = SG90
shaft_x = -iw - CAGE_WALL - 0.6 - sv["W"] / 2
shaft_y = front + 5
body_y1 = shaft_y + sv["shaft_from_end"]
body_y0 = body_y1 - sv["L"]
body_cy = (body_y0 + body_y1) / 2
pad = box(shaft_x - sv["W"] / 2 - 5, -iw, body_cy - 19, body_cy + 19, pz0, pz1)
riser = box(-20, 20, 85, CAGE_BACK_Y + CAGE_WALL, pz1 - 0.01, lz0)
cage = union(walls + [plate, pad, riser])
cut = [
    box(shaft_x - sv["W"] / 2 - 0.3, shaft_x + sv["W"] / 2 + 0.3, body_y0 - 0.3, body_y1 + 0.3, pz0 - 1, pz1 + 1),
    cyl_z(shaft_x, body_cy - sv["tab_hole_pitch"] / 2, pz0 - 1, pz1 + 1, 1.0),
    cyl_z(shaft_x, body_cy + sv["tab_hole_pitch"] / 2, pz0 - 1, pz1 + 1, 1.0),
]
for x, y in CAGE_SCREWS:
    cut.append(cyl_z(x, y, pz0 - 1, lz0 + 1, M3 / 2))
    cut.append(cyl_z(x, y, pz0 - 1, pz0 + 3, 3.2, 6))   # pocket for an M3 nut, from below
# window in the top plate over the ball, so you can see it is caught
cut.append(rounded_plate(-iw + 4, iw - 4, CAGE_BACK_Y + CAGE_WALL + 2, front + 10, pz0 - 1, pz1 + 1, 4))
claw_cage = cage - union(cut)

# ======================= GATE ARM =======================
arm_len = 2 * iw + CAGE_WALL + (-iw - CAGE_WALL - shaft_x) + 6
arm_z = pz0 - 9   # sits on the servo horn, below the plate
hub = cyl_z(shaft_x, shaft_y, arm_z - 3, arm_z, 5.5)
bar = box(shaft_x, shaft_x + arm_len, shaft_y - 4, shaft_y + 4, arm_z - 3, arm_z)
drop = box(shaft_x + 18, shaft_x + arm_len, shaft_y - 1.5, shaft_y + 1.5, arm_z - 8, arm_z)  # pushes at ball height
gate = hub + bar + drop
gcut = [cyl_z(shaft_x, shaft_y, arm_z - 4, arm_z + 1, 2.6)]  # horn hub / screw
for d in (7, 10.5, 14):
    gcut.append(cyl_z(shaft_x + d, shaft_y, arm_z - 4, arm_z + 1, 0.8))  # horn screw holes, pick the ones that fit
gate_arm = gate - union(gcut)

# ======================= SPACERS =======================
def tube(x, y, z0, z1, od=7, idd=M3):
    return cyl_z(x, y, z0, z1, od / 2, 6) - cyl_z(x, y, z0 - 1, z1 + 1, idd / 2, 24)

spacers = []
spacers += [tube(x, y, lz1, tz0) for x, y in STANDOFF_XY]                       # 4 x 37 mm
spacers += [tube(x, y, sb["top_z"], lz0, od=6) for x, y in sb_holes]            # 4 x 25.5 mm
spacers += [tube(s * 9, COLOUR_Y, COLOUR_TOP_Z, lz0, od=5, idd=2.6) for s in (-1, 1)]  # 2 x 29.5 mm, M2.5
spacers_all = union(spacers)

# ======================= CASTER SPACER =======================
CASTER_H = 21   # height of YOUR ball caster from floor to its mounting face (CY-15A type = 21)
cs_h = LOW_Z - CASTER_H
caster_spacer = rounded_plate(-26, 26, CASTER_Y - 12, CASTER_Y + 12, CASTER_H, LOW_Z, 4)
caster_spacer = caster_spacer - union([slot_z(s * 8, CASTER_Y, s * 22, CASTER_Y, CASTER_H - 1, LOW_Z + 1, M3 / 2) for s in (-1, 1)])

PARTS = {
    "lower_deck": (lower_deck, "flip"),
    "top_deck": (top_deck, "flat"),
    "claw_cage": (claw_cage, "flip"),
    "gate_arm": (gate_arm, "flat"),
    "spacers": (spacers_all, "flat"),
    "caster_spacer": (caster_spacer, "flat"),
}

def to_trimesh(m):
    mesh = m.to_mesh()
    return trimesh.Trimesh(vertices=np.asarray(mesh.vert_properties)[:, :3], faces=np.asarray(mesh.tri_verts), process=False)

report = {}
for name, (m, mode) in PARTS.items():
    t = to_trimesh(m)
    t.export(os.path.join(OUT, f"assembled_{name}.stl"))
    p = t.copy()
    if mode == "flip":
        p.apply_transform(trimesh.transformations.rotation_matrix(np.pi, [1, 0, 0]))
    p.apply_translation(-p.bounds[0])
    if name == "spacers":   # lay spacers out in a row on the bed
        pieces = p.split(only_watertight=False)
        laid, x = [], 0
        for pc in sorted(pieces, key=lambda q: -q.extents[2]):
            pc = pc.copy(); pc.apply_translation(-pc.bounds[0] + [x, 0, 0]); x += pc.extents[0] + 4; laid.append(pc)
        p = trimesh.util.concatenate(laid)
    p.export(os.path.join(OUT, f"print_{name}.stl"))
    report[name] = dict(size_mm=[round(v, 1) for v in p.extents], volume_cm3=round(m.volume() / 1000, 1),
                        watertight=bool(t.is_watertight), pieces=m.num_prop_vert() and None)

meta = dict(shaft=[shaft_x, shaft_y, arm_z], ball=[0, (CAGE_BACK_Y + CAGE_WALL + front) / 2, BALL_D / 2],
            caster_y=CASTER_Y, colour_y=COLOUR_Y, sensor=sb, low_z=LOW_Z, top_z=tz1, axle=[AXLE_Y, AXLE_Z],
            motor_x=MOTOR_X, standoffs=STANDOFF_XY, mainboard=mb, cage_front=front)
json.dump(dict(parts=report, meta=meta), open(os.path.join(OUT, "report.json"), "w"), indent=1)
print(json.dumps(report, indent=1))
