import MPKMC
import numpy as np

def FSFCCEnergy(kmc, types):
    fsfcc = MPKMC.fsfcc(types[0])
    return fsfcc.energy(kmc, types)

if __name__ == "__main__":
    Ntry = 20000
    T = range(1100, 800, -20)
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (29, 29, 29, 29)
    pv = ((3.615, 0, 0), (0, 3.615, 0), (0, 0, 3.615))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5),\
        (0.5, 0, -0.5), (1.0, 0, 0), (-1.0, 0, 0), (0, 1.0, 0), (0, -1.0, 0),\
        (0, 0, 1.0), (0, 0, -1.0))
    jclu = (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0)
    kmc = MPKMC.new(4, 10, 10, 10, 19)
    kmc.kb = 86.1735e-6 # ev/K
    kmc.rand_seed = 12345
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, jclu)
    kmc.calc_rot_index(5.0, 1.0e-6)
    kmc.add_solute_random(30, 0, 1)
    kmc.total_energy(FSFCCEnergy)
    print kmc.tote, kmc.ntable
    for temp in T:
        njump, update = kmc.jump(Ntry, temp, FSFCCEnergy)
        print kmc.totmcs, temp, njump, kmc.tote, kmc.ntable
    kmc.write_table('fsfcc.etb')
    kmc.write('fsfcc.mpkmc', 8)
