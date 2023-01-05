import MPKMC
import subprocess
import os
import numpy as np
import math
import pandas as pd

Symbols = {1: 'H', 2: 'He', 3: 'Li', 4: 'Be', 5: 'B', 6: 'C',
           7: 'N', 8: 'O', 9: 'F', 10: 'Ne', 11: 'Na', 12: 'Mg',
           13: 'Al', 14: 'Si', 15: 'P', 16: 'S', 17: 'Cl', 18: 'Ar',
           19: 'K', 20: 'Ca', 21: 'Sc', 22: 'Ti', 23: 'V', 24: 'Cr',
           25: 'Mn', 26: 'Fe', 27: 'Co', 28: 'Ni', 29: 'Cu', 30: 'Zn',
           31: 'Ga', 32: 'Ge', 33: 'As', 34: 'Se', 35: 'Br', 36: 'Kr',
           37: 'Rb', 38: 'Sr', 39: 'Y', 40: 'Zr', 41: 'Nb', 42: 'Mo',
           43: 'Tc', 44: 'Ru', 45: 'Rh', 46: 'Pd', 47: 'Ag', 48: 'Cd'}

def writeIN(types, cluster):
    with open('qedefault.in', mode='r') as fin:
        default = fin.read()
    line = 'nat = %d' % (13 - types.count(0))
    mod = default.replace('nat = 32', line)
    with open('tmp.in', mode='w') as fout:
        fout.write(mod)
        for i in range(len(types)):
            if types[i] != 0:
                x = cluster[i][0]/2.0 + 0.5
                y = cluster[i][1]/2.0 + 0.5
                z = cluster[i][2]/2.0 + 0.5
                line = ' %s %3.2f %3.2f %3.2f\n' % (Symbols[types[i]], x, y, z)
                fout.write(line)

def readOUT(outfile):
    f = open(outfile, 'r')
    for line in f:
        if line.find('!    total energy              =') == 0:
            sp = line.split()
            f.close()
            return float(sp[4])
    f.close()
    return 0.0

# def calcQE(kmc, types):
#     table = [[13, 'Al', -4.77069668], [40, 'Zr', -99.335965320625],[3, 'Li',-14.469288874375],[12,'Mg', -126.44307896625],[0,'vac',0],[14, 'Si', -11.0243531],[29, 'Cu',  -49.818497277]]
#     writeIN(types, kmc.cluster, table)
#     str2 =MPKMC.types2string(types)
#     print(str2)
#     if os.path.exists(str2+'.out'):
#     	tote = readOUT(str2+'.out')
#     else:
#     	outfile = '%s.out' % (MPKMC.types2string(types))
#     	cmd = './pw.x < tmp.in > %s' % (outfile)
#     	print(cmd)
#     	subprocess.call(cmd, shell=True)
#     	tote = readOUT(outfile)
#     totc=0
#     for tp in types:
#         if tp != 0:
#             tote =tote -isoEnergy(tp, table)
#             totc = totc+1
#     #tote=tote*13.6058
#     print (types, totc, tote/totc)
#     return tote/totc

def calcQE(kmc, types):
    energies = { 3: -14.469288874375,
                 12: -126.44307896625,
                 13: -4.77069668,
                 14: -11.0243531,
                 29: -49.818497277,
                 40: -99.335965320625, }
    writeIN(types, kmc.cluster)
    outfile = '%s.out' % (MPKMC.types2string(types))
    if not os.path.exists(outfile):
        cmd = './pw.x < tmp.in > %s' % (outfile)
        print(cmd)
        subprocess.call(cmd, shell=True)
    tote = readOUT(outfile)
    tote = tote - 19 * energies[13]
    totc = 19
    for tp in types:
        if tp != 0:
            tote = tote - energies[tp]
            totc = totc + 1
    print(types, totc, tote/totc)
    return tote/totc

if __name__ == "__main__":
    Ntry = 10000
    T = range(800, 700, -1)
    etbfile = 'Al-Si_13.etb'
    resfile = 'Al-Si_13.mpkmc'
    Kb = 86.1735e-6; # ev/K
    Ry = 13.6058; # ev
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
    #kmc.add_solute_random(6, 0, 1)
    kmc.grid_energy(calcQE)
    kmc.write_table(etbfile)
    print(kmc.tote, kmc.ntable)
    print('(totmcs, temp, ntry, njump, table_update, ntable, tote, tott, time) ngroup')
    for temp in T:
        ret = kmc.jump(Ntry, temp, calcQE)
        kmc.write_table(etbfile)
        print(ret, kmc.find_solute_group(1.42))
    kmc.write(resfile, 8)
