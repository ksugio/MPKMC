import MPKMC
import numpy as np

def FSFCCEnergy(kmc, types):
    fsfcc = MPKMC.fsfcc(types[0])
    return fsfcc.energy(kmc, types)

if __name__ == "__main__":
    Ntry = 100000
    T = range(1000, 500, -50)
    Kb = 86.1735e-6 # ev/K
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5),\
        (0.5, 0, -0.5))
    kmc = MPKMC.new(4, 10, 10, 10, 13, 1000, 1000, 1000)
    kmc.rand_seed = 12345
    kmc.set_unitcell(uc)
    kmc.set_cluster(clu)
    kmc.calc_rot_index(5.0)
    kmc.set_solvent(29)
    kmc.add_solute_random(30, 0, 1)
    kmc.total_energy(FSFCCEnergy)
    print kmc.tote
    for temp in T:
        njump, update = kmc.jump(Ntry, Kb*temp, FSFCCEnergy)
        print temp, njump, kmc.tote
    kmc.write_table('fsfcc.etb')
    kmc.write('fsfcc.mpkmc', 8)
