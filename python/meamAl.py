import MPKMC
import pandas as pd
from frac import fractureEnergies

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

def summaryFE(kmc):
    kmc2 = MPKMC.copy(kmc, 0, 0)
    planes = [[1, 1, 1, -19.5, 0.6], [1, 1, -1, 0.5, 0.6], [1, -1, 1, -9.5, 0.6], [-1, 1, 1, -9.5, 0.6]]
    shift = 200
    area = kmc2.fracture_area(1, 1, 1, -19.99999)
    fes = fractureEnergies(kmc2, planes, shift, MEAMEnergy, MEAM111Energy)
    df = pd.DataFrame(fes, columns=['Energy'])
    ev = 1.6021773300241e-19
    df['EnergyJsqM'] = df['Energy'] * ev / (area * 1.0e-20)
    print('Fracture Energy in J/m-2')
    print(df['EnergyJsqM'].describe())

if __name__ == "__main__":
    Ntry = 10000
    T = range(800, 0, -10)
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (13, 13, 13, 13)
    pv = ((4.04466, 0, 0), (0, 4.04466, 0), (0, 0, 4.04466))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5))
    kmc = MPKMC.new(4, 10, 10, 20, 13)
    kmc.kb = 86.1735e-6; # ev/K
    kmc.freq = 1.0e13
    kmc.rand_seed = 123456
    kmc.table_use = False
    kmc.event_record = True
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    kmc.add_solute_random(100, 29, 1)
    summaryFE(kmc)
    kmc.grid_energy(MEAMEnergy)
    print('(totmcs, temp, ntry, njump, table_update, ntable, tote, tott, time) ngroup')
    for temp in T:
      ret = kmc.jump(Ntry, temp, MEAMEnergy)
      print(ret, kmc.find_solute_group(0.71))
    summaryFE(kmc)
    kmc.write('meamAl.mpkmc', 8)





