#!/usr/bin/env python3
import sys
import math
from dataclasses import dataclass
from typing import Optional, Tuple, List

# ---------------------------- Config ----------------------------
FOCAL_LENGTH = 1600.0  # mm
MIRROR_HEIGHT = 150.0  # mm

# 54 shell Yp_min values (mm)
YP_MIN_LIST = [
    174.2,169.19,164.39,159.60,155.04,150.61,146.32,142.12,138.07,134.15,130.28,126.55,
    122.98,119.45,116.02,112.72,109.46,106.34,103.36,100.41, 97.52, 94.69, 91.95, 89.36,
    86.80, 84.34, 81.96, 79.59, 77.30, 75.07, 72.93, 70.87, 68.85, 66.85, 64.92, 63.07,
    61.32, 59.53, 57.83, 56.15, 54.58, 53.00, 51.52, 50.05, 48.59, 47.22, 45.84, 44.56,
    43.30, 42.09, 40.87, 39.67, 38.47, 37.24
]

# Sensor plane (ax + by + cz + d = 0): z = 1610.4 mm
PLANE_N = (0.0, 0.0, 1.0)
PLANE_D = -1610.4

# Optional pixelization (for reporting pixel indices)
SENSOR_PIXEL_SIZE_MM = 7.5   # set None to skip pixel index
SENSOR_RES = 1024            # square; only used to compute (ix,iy) if size given

# Trace limits
EPS_T = 1e-4
MAX_STEPS = 8

# ------------------------- Math helpers -------------------------
def dot(a,b): return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
def add(a,b): return (a[0]+b[0], a[1]+b[1], a[2]+b[2])
def sub(a,b): return (a[0]-b[0], a[1]-b[1], a[2]-b[2])
def mul(a,s): return (a[0]*s, a[1]*s, a[2]*s)
def length(v): return math.sqrt(dot(v,v))
def norm(v):
    L = length(v)
    if L == 0: return (0.0,0.0,0.0)
    return (v[0]/L, v[1]/L, v[2]/L)

def reflect(i, n):  # reflect vector i about unit normal n
    k = 2.0 * dot(i, n)
    return (i[0]-k*n[0], i[1]-k*n[1], i[2]-k*n[2])

def clamp(x,a,b): return max(a, min(b, x))

def solve_quadratic(A: float, B: float, C: float) -> Optional[Tuple[float,float]]:
    """ Numerically-stable quadratic solver; returns (t0<=t1) or None """
    if abs(A) < 1e-16:
        if abs(B) < 1e-16: return None
        t = -C / B
        return (t,t)
    disc = B*B - 4.0*A*C
    if disc < 0.0: return None
    sd = math.sqrt(disc)
    q = -0.5 * (B + (sd if B >= 0 else -sd))
    t0 = q / A
    t1 = C / q if q != 0 else t0
    if t0 > t1: t0, t1 = t1, t0
    return (t0, t1)

# ------------------------- Shell model --------------------------
@dataclass
class Paraboloid:
    p: float
    zmin: float
    zmax: float
    idx: int   # shell index

@dataclass
class Hyperboloid:
    a: float
    b: float
    c: float
    zmin: float
    zmax: float
    idx: int   # shell index

@dataclass
class Scene:
    paras: List[Paraboloid]
    hypos: List[Hyperboloid]

def build_shells() -> Scene:
    paras: List[Paraboloid] = []
    hypos: List[Hyperboloid] = []
    c = FOCAL_LENGTH * 0.5

    for i, Ypmin in enumerate(YP_MIN_LIST):
        theta = math.asin(Ypmin / FOCAL_LENGTH) * 0.25
        Xp_min = FOCAL_LENGTH * math.cos(4.0*theta) + 2.0*c
        Xp_max = Xp_min + MIRROR_HEIGHT

        p = Ypmin * math.tan(theta)
        # Yp_max available if needed:
        # Yp_max = math.sqrt(max(0.0, p * (2.0*Xp_max + p)))

        paras.append(Paraboloid(p=p, zmin=Xp_min, zmax=Xp_max, idx=i))

        a = FOCAL_LENGTH * (2.0*math.cos(2.0*theta) - 1.0) * 0.5
        b = math.sqrt(max(0.0, c*c - a*a))
        Xh_max = Xp_min
        Xh_min = Xp_min - MIRROR_HEIGHT

        hypos.append(Hyperboloid(a=a, b=b, c=c, zmin=Xh_min, zmax=Xh_max, idx=i))

    return Scene(paras=paras, hypos=hypos)

# ----------------------- Intersections --------------------------
def intersect_plane(ro, rd, n=PLANE_N, d=PLANE_D) -> Optional[Tuple[float,Tuple[float,float,float]]]:
    denom = dot(n, rd)
    if abs(denom) < 1e-9: return None
    t = -(dot(n, ro) + d) / denom
    if t > EPS_T:
        return t, add(ro, mul(rd, t))
    return None

def intersect_paraboloid(ro, rd, P: Paraboloid) -> Optional[Tuple[float,Tuple[float,float,float],Tuple[float,float,float]]]:
    # Model: x^2 + y^2 - 2 p z - p^2 = 0  (opens +Z); inward normal = normalize([x,y,-p])
    A = rd[0]*rd[0] + rd[1]*rd[1]
    B = 2.0*(ro[0]*rd[0] + ro[1]*rd[1] - P.p*rd[2])
    C = ro[0]*ro[0] + ro[1]*ro[1] - 2.0*P.p*ro[2] - P.p*P.p
    print(f"A P_{P.idx}: {A}")
    print(f"B P_{P.idx}: {B}")
    print(f"C P_{P.idx}: {C}")
    ts = solve_quadratic(A,B,C)
    if ts is None: return None
    t0, t1 = ts
    hit_t = None
    for t in (t0, t1):
        if t > EPS_T:
            z = ro[2] + rd[2]*t
            if P.zmin <= z <= P.zmax:
                if hit_t is None or t < hit_t: hit_t = t
    if hit_t is None: return None
    pHit = add(ro, mul(rd, hit_t))
    nHit = norm((pHit[0], pHit[1], -P.p))  # inward
    return hit_t, pHit, nHit

def intersect_hyperboloid(ro, rd, H: Hyperboloid) -> Optional[Tuple[float,Tuple[float,float,float],Tuple[float,float,float]]]:
    # x^2/b^2 + y^2/b^2 - (z-c)^2/a^2 = -1 ; inward normal = normalize([x/b^2, y/b^2, -(z-c)/a^2])
    a2 = H.a*H.a
    b2 = H.b*H.b
    A = (rd[0]*rd[0] + rd[1]*rd[1])/b2 - (rd[2]*rd[2])/a2
    B = 2.0*((ro[0]*rd[0] + ro[1]*rd[1])/b2 - ((ro[2]-H.c)*rd[2])/a2)
    C = (ro[0]*ro[0] + ro[1]*ro[1])/b2 - ((ro[2]-H.c)*(ro[2]-H.c))/a2 + 1.0
    print(f"A H_{H.idx}: {A}")
    print(f"B H_{H.idx}: {B}")
    print(f"C H_{H.idx}: {C}")
 
    ts = solve_quadratic(A,B,C)
    if ts is None: return None
    t0, t1 = ts
    hit_t = None
    for t in (t0, t1):
        if t > EPS_T:
            z = ro[2] + rd[2]*t
            if H.zmin <= z <= H.zmax:
                if hit_t is None or t < hit_t: hit_t = t
    if hit_t is None: return None
    pHit = add(ro, mul(rd, hit_t))
    nHit = norm((pHit[0]/b2, pHit[1]/b2, -(pHit[2]-H.c)/a2))  # inward
    return hit_t, pHit, nHit

# ------------------------ Tracing logic -------------------------
@dataclass
class HitRecord:
    kind: str            # "paraboloid"/"hyperboloid"/"plane"
    shell: Optional[int] # shell index for mirrors
    t: float
    p: Tuple[float,float,float]
    n: Optional[Tuple[float,float,float]]  # None for plane
    inc_angle_deg: Optional[float]         # None for plane
    note: str

def trace_ray(ro, rd, scene: Scene) -> List[HitRecord]:
    """ Trace until sensor or termination. Reflect only on inward face. """
    ro = tuple(ro)
    rd = norm(tuple(rd))
    history: List[HitRecord] = []

    for step in range(MAX_STEPS):
        nearest_t = float('inf')
        best = None  # (kind, shell_idx, t, p, n)

        # Check all mirrors
        for P in scene.paras:
            res = intersect_paraboloid(ro, rd, P)
            if res:
                t, p, n = res
                if t < nearest_t:
                    nearest_t = t; best = ("paraboloid", P.idx, t, p, n)
        for H in scene.hypos:
            res = intersect_hyperboloid(ro, rd, H)
            if res:
                t, p, n = res
                if t < nearest_t:
                    nearest_t = t; best = ("hyperboloid", H.idx, t, p, n)

        # Sensor plane
        plane_hit = intersect_plane(ro, rd, PLANE_N, PLANE_D)
        if plane_hit:
            tP, pP = plane_hit
            if tP < nearest_t:
                # Reached plane first -> stop
                note = "sensor within bounds? "
                if SENSOR_PIXEL_SIZE_MM and SENSOR_RES:
                    halfW = SENSOR_RES * SENSOR_PIXEL_SIZE_MM * 0.5
                    halfH = halfW
                    inside = (-halfW <= pP[0] <= halfW) and (-halfH <= pP[1] <= halfH)
                    note += "yes" if inside else "no"
                    if inside:
                        ix = int(math.floor((pP[0] + halfW) / SENSOR_PIXEL_SIZE_MM))
                        iy = int(math.floor((halfH - pP[1]) / SENSOR_PIXEL_SIZE_MM))
                        note += f"  (pixel ix={ix}, iy={iy})"
                history.append(HitRecord(kind="plane", shell=None, t=tP, p=pP, n=None, inc_angle_deg=None, note=note))
                break

        if best is None:
            # Nothing ahead
            history.append(HitRecord(kind="miss", shell=None, t=float('inf'), p=ro, n=None, inc_angle_deg=None, note="no further intersections"))
            break

        kind, shell_idx, t, p, n = best
        # Check side: reflect only if hitting inward face -> dot(n, rd) > 0
        side_dot = dot(n, rd)
        inc_deg = math.degrees(math.acos(clamp(abs(dot(n, (-rd[0],-rd[1],-rd[2]))), 0.0, 1.0)))

        if side_dot > 0.0:
            history.append(HitRecord(kind=kind, shell=shell_idx, t=t, p=p, n=n,
                                     inc_angle_deg=inc_deg, note="reflect (inward side)"))
            # reflect and continue
            rd = norm(reflect(rd, n))
            ro = add(p, mul(rd, 1e-4))  # offset
        else:
            history.append(HitRecord(kind=kind, shell=shell_idx, t=t, p=p, n=n,
                                     inc_angle_deg=inc_deg, note="absorbed (hit outward side)"))
            break

    return history

# --------------------------- CLI ----------------------------
def fmt3(v): return f"({v[0]:.6f}, {v[1]:.6f}, {v[2]:.6f})"

def main():
    if len(sys.argv) == 7+1:
        rx, ry, rz, dx, dy, dz = map(float, sys.argv[1:7+1])
    elif len(sys.argv) == 7:
        rx, ry, rz, dx, dy, dz = map(float, sys.argv[1:7])
    else:
        print("Usage: python debug_ray.py rx ry rz  dx dy dz")
        print("Example (on-axis, from z=5000): python debug_ray.py 0 0 5000   0 0 -1")
        return

    scene = build_shells()
    ro = (rx, ry, rz)
    rd = (dx, dy, dz)

    print("=== Ray Debug Trace ===")
    print(f"origin  = {fmt3(ro)}")
    print(f"dir     = {fmt3(norm(rd))}  (normalized)")
    print(f"sensor  = plane z = {-PLANE_D:.3f} mm")
    print(f"shells  = {len(scene.paras)} paraboloids + {len(scene.hypos)} hyperboloids")
    print("------------------------------------------------------------")

    hist = trace_ray(ro, rd, scene)

    par_idx = None
    hyp_idx = None
    sensor_hit = None

    for i, h in enumerate(hist, 1):
        if h.kind == "plane":
            sensor_hit = h
            print(f"[{i}] SENSOR  t={h.t:.6f}  p={fmt3(h.p)}  {h.note}")
        elif h.kind == "paraboloid":
            par_idx = h.shell if par_idx is None else par_idx
            print(f"[{i}] PAR#{h.shell:02d}  t={h.t:.6f}  p={fmt3(h.p)}  n={fmt3(h.n)}  inc={h.inc_angle_deg:.3f}°  -> {h.note}")
        elif h.kind == "hyperboloid":
            hyp_idx = h.shell if hyp_idx is None else hyp_idx
            print(f"[{i}] HYP#{h.shell:02d}  t={h.t:.6f}  p={fmt3(h.p)}  n={fmt3(h.n)}  inc={h.inc_angle_deg:.3f}°  -> {h.note}")
        elif h.kind == "miss":
            print(f"[{i}] MISS    {h.note}")
        else:
            print(f"[{i}] {h.kind}  t={h.t:.6f}")

    print("------------------------------------------------------------")
    if par_idx is not None: print(f"First paraboloid shell hit: {par_idx}")
    if hyp_idx is not None: print(f"First hyperboloid shell hit: {hyp_idx}")
    if sensor_hit:
        print(f"Hit sensor at {fmt3(sensor_hit.p)}")
    else:
        print("Did not reach the sensor.")

if __name__ == "__main__":
    main()
