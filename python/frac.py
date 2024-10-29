import MPKMC
import math
import pandas as pd
import matplotlib.pyplot as plt

FSFcc = MPKMC.fsfcc()

def FSFccEnergy(kmc, types):
    return FSFcc.energy(kmc, types)

def fccCu():
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (29, 29, 29, 29)
    pv = ((3.615, 0, 0), (0, 3.615, 0), (0, 0, 3.615))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5),\
        (0.5, 0, -0.5), (1.0, 0, 0), (-1.0, 0, 0), (0, 1.0, 0), (0, -1.0, 0),\
        (0, 0, 1.0), (0, 0, -1.0))
    kmc = MPKMC.new(4, 10, 10, 20, 19)
    kmc.kb = 86.1735e-6  # ev/K
    kmc.rand_seed = 12345
    kmc.event_record = True
    kmc.table_use = True
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    return kmc

def fractureEnergy(kmc, plane, func):
    kmc.grid_energy(func)
    kmc1 = MPKMC.copy(kmc)
    c1 = kmc1.change_type_by_plane(plane[0], plane[1], plane[2], plane[3], 0)
    kmc1.grid_energy(func)
    kmc2 = MPKMC.copy(kmc)
    c2 = kmc2.change_type_by_plane(-plane[0], -plane[1], -plane[2], -plane[3], 0)
    kmc2.grid_energy(func)
    if c1 + c2 != kmc.ntot:
        print('Number of atoms not match, invalid plane difinition.')
    return kmc1.tote + kmc2.tote - kmc.tote, c1, c2

def HeronArea(p0, p1, p2):
    a = math.sqrt(math.pow(p0[0] - p1[0], 2) + math.pow(p0[1] - p1[1], 2) + math.pow(p0[2] - p1[2], 2))
    b = math.sqrt(math.pow(p1[0] - p2[0], 2) + math.pow(p1[1] - p2[1], 2) + math.pow(p1[2] - p2[2], 2))
    c = math.sqrt(math.pow(p2[0] - p0[0], 2) + math.pow(p2[1] - p0[1], 2) + math.pow(p2[2] - p0[2], 2))
    s = (a + b + c) / 2
    return math.sqrt(s * (s - a) * (s - b) * (s - c))

def Intersection(kmc, plane, line):
    lx = line[0] * kmc.size[0]
    ly = line[1] * kmc.size[1]
    lz = line[2] * kmc.size[2]
    if lx < 0 and plane[0] != 0:
        x = -(plane[1] * ly + plane[2] * lz + plane[3]) / plane[0]
        if x < -lx and x >= 0:
            return x, ly, lz
    if ly < 0 and plane[1] != 0:
        y = -(plane[0] * lx + plane[2] * lz + plane[3]) / plane[1]
        if y < -ly and y >= 0:
            return lx, y, lz
    if lz < 0 and plane[2] != 0:
        z = -(plane[0] * lx + plane[1] * ly + plane[3]) / plane[2]
        if z < -lz and z >= 0:
            return lx, ly, z
    return None, None, None

def fractureArea(kmc, plane):
    lines = [[-1, 0, 0], [0, -1, 0], [0, 0, -1],
            [-1, 1, 0], [1, -1, 0], [1, 0, -1],
            [-1, 1, 1], [1, -1, 1], [1, 1, -1],
            [-1, 0, 1], [0, -1, 1], [0, 1, -1]]
    pts = []
    for line in lines:
        x, y, z = Intersection(kmc, plane, line)
        if x != None and y != None and z != None:
            pts.append([x, y, z])
    pts2 = []
    spt = pts.pop(0)
    pts2.append(spt)
    while len(pts) > 0:
        dis = 1.0e23
        for i, pt in enumerate(pts):
            dx = pt[0] - spt[0]
            dy = pt[1] - spt[1]
            dz = pt[2] - spt[2]
            d = math.sqrt(dx * dx + dy * dy + dz * dz)
            if d < dis:
                dis = d
                id = i
        spt = pts.pop(id)
        if dis > 1.0e-3:
            pts2.append(spt)
    area = 0
    for i in range(len(pts2) - 2):
        area += HeronArea(pts2[0], pts2[i + 1], pts2[i + 2])
    return area

def fractureEnergies(kmc, planes, shfit, func):
    areas = []
    for plane in planes:
        areas.append(fractureArea(kmc, plane))
    feas = []
    layers = int(kmc.ntot / shift)
    for i in range(layers):
        for plane, area in zip(planes, areas):
            fe, c1, c2 = fractureEnergy(kmc, plane, func)
            feas.append([fe, area])
            print('%d layer,(%d %d %d) plane, %d %d, %f energy, %f area, %f energy/area' % (i, plane[0], plane[1], plane[2], c1, c2, fe, area, fe / area))
        kmc.shift_grid(shift)
    return feas

if __name__ == "__main__":
    kmc = fccCu()
    kmc.add_solute_random(30, 0, 1)
    alat = 3.615
    csvname = 'fracCu.csv'
    planes = [[1, 1, 1, -19.999999], [1, 1, -1, 0.000001], [1, -1, 1, -9.999999], [-1, 1, 1, -9.999999]]
    shift = 200
    feas = fractureEnergies(kmc, planes, shift, FSFccEnergy)
    df = pd.DataFrame(feas, columns=['Energy', 'Area'])
    ev = 1.6021773300241e-19
    df['EnergyJsqM'] = df['Energy'] * ev / df['Area'] / (alat * alat * 1.0e-20)
    print(df['EnergyJsqM'].describe())
    df.to_csv(csvname, index=False)
    plt.hist(df['EnergyJsqM'], bins=16)
    plt.show()
