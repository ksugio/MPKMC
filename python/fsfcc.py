import MPKMC
import numpy as np

def FSFCCEnergy(kmc, types):
    fsfcc = MPKMC.fsfcc(types[0])
    return fsfcc.energy(kmc, types)

if __name__ == "__main__":
    Ntry = 10000
    T = range(1000, 800, -50)
    Kb = 86.1735e-6 # ev/K
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (29, 29, 29, 29)
    pv = ((3.615, 0, 0), (0, 3.615, 0), (0, 0, 3.615))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5),\
        (0.5, 0, -0.5), (1.0, 0, 0), (-1.0, 0, 0), (0, 1.0, 0), (0, -1.0, 0),\
        (0, 0, 1.0), (0, 0, -1.0))
    kmc = MPKMC.new(4, 10, 10, 10, 19, 100, 1000, 10000)
    kmc.rand_seed = 12345
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu)
    kmc.calc_rot_index(5.0, 1.0e-6)
    kmc.add_solute_random(30, 0, 1)
    kmc.total_energy(FSFCCEnergy)
    print kmc.tote, kmc.ntable
    for temp in T:
        njump, update = kmc.jump(Ntry, Kb*temp, FSFCCEnergy)
        print temp, njump, kmc.tote, kmc.ntable
    kmc.write_table('fsfcc.etb')
    kmc.write('fsfcc.mpkmc', 8)
