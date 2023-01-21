import MPKMC
import qetool
import subprocess
import os

def calcQE(kmc, types):
    isolates = {0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    ebonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0 }
    qetool.writeQEin(types, kmc.cluster, 'qehead.in', 'tmp.in', fill_type=13)
    outfile = '%s/%s.out' % ('AlSi60', MPKMC.types2string(types))
    if not os.path.exists(outfile):
        cmd = './pw.x < tmp.in > %s' % (outfile)
        print(cmd)
        subprocess.call(cmd, shell=True)
    data = qetool.readQEout(outfile)
    clue = qetool.ClusterEnergy(data, isolates, ebonds)
    return clue

if __name__ == "__main__":
    Ntry = 10000
    T = range(800, 700, -1)
    etbfile = 'Al-Si-V.etb'
    resfile = 'Al-Si-V.mpkmc'
    Kb = 86.1735e-6 # ev/K
    Ry = 13.6058 # ev
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (13, 13, 13, 13)
    pv = ((4.04360, 0, 0), (0, 4.04360, 0), (0, 0, 4.04360))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5))
    kmc = MPKMC.new(4, 10, 10, 10, 13, 100, 1000, 10000)
    kmc.kb = Kb/Ry
    kmc.freq = 1.0e13 # frequency of atom
    kmc.mige = 0.66 # vacancy migration energy in Aluminum
    kmc.rand_seed = 12345
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    if os.path.exists(etbfile):
        kmc.read_table(etbfile)
    kmc.add_solute_random(10, 14, 1)
    kmc.add_solute_random(6, 0, 1)
    kmc.grid_energy(calcQE)
    kmc.write_table(etbfile)
    print(kmc.tote, kmc.ntable)
    print('(totmcs, temp, ntry, njump, table_update, ntable, tote, tott, time) ngroup')
    for temp in T:
        ret = kmc.jump(Ntry, temp, calcQE)
        kmc.write_table(etbfile)
        print(ret, kmc.find_solute_group(1.42))
    kmc.write(resfile, 8)
