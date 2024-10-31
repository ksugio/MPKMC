import MPKMC
import math
import pandas as pd
import matplotlib.pyplot as plt

FSFcc = MPKMC.fsfcc()

def FSFccEnergy(kmc, types):
    return FSFcc.energy(kmc, types)

def FSFccCu():
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

Meam = MPKMC.meam()

def MEAMEnergy(kmc, types):
    return Meam.energy(kmc, types)

Meam111 = MPKMC.meam()
Meam111.Zd = 9
Meam111.S1 = 6
Meam111.S2 = 3 / 2
Meam111.S3 = 15 / 4

def MEAM111Energy(kmc, types):
    return Meam111.energy(kmc, types)

def MEAMFccAl():
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (13, 13, 13, 13)
    pv = ((4.04466, 0, 0), (0, 4.04466, 0), (0, 0, 4.04466))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5))
    kmc = MPKMC.new(4, 10, 10, 20, 13)
    kmc.rand_seed = 123456
    kmc.table_use = False
    kmc.event_record = True
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    return kmc

def fractureEnergies(kmc, planes, shift, perf_func, frac_func):
    fes = []
    layers = int(kmc.ntot / shift)
    for i in range(layers):
        for plane in planes:
            fe = kmc.fracture_energy(plane[0], plane[1], plane[2], plane[3], plane[4], perf_func, frac_func)
            fes.append(fe)
            print('%d layer,(%d %d %d) plane, %f energy' % (i, plane[0], plane[1], plane[2], fe))
        kmc.shift_grid(shift)
    return fes

if __name__ == "__main__":
    kmc = MEAMFccAl()
    #kmc = FSFccCu()
    kmc.add_solute_random(100, 26, 1)
    csvname = 'fracCu.csv'
    planes = [[1, 1, 1, -19.5, 0.6], [1, 1, -1, 0.5, 0.6], [1, -1, 1, -9.5, 0.6], [-1, 1, 1, -9.5, 0.6]]
    shift = 200
    area = kmc.fracture_area(1, 1, 1, -19.99999)
    fes = fractureEnergies(kmc, planes, shift, MEAMEnergy, MEAM111Energy)
    df = pd.DataFrame(fes, columns=['Energy'])
    ev = 1.6021773300241e-19
    df['EnergyJsqM'] = df['Energy'] * ev / (area * 1.0e-20)
    print(df['EnergyJsqM'].describe())
    df.to_csv(csvname, index=False)
    plt.hist(df['EnergyJsqM'], bins=16)
    plt.show()
