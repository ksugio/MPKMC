import MPKMC
import subprocess
import os
import numpy as np
import math as m

DefaultIn =\
"&control\n\
 calculation = 'scf'\n\
 prefix='Al-Si'\n\
/\n\
&system\n\
 ibrav = 1\n\
 celldm(1) = 15.42017132\n\
 nat = 13\n\
 ntyp = 2\n\
 ecutwfc = 30.0\n\
 ecutrho = 150.0\n\
 occupations = 'smearing'\n\
 smearing = 'm-p'\n\
 degauss = 0.001\n\
 assume_isolated = 'mp'\n\
/\n\
&electrons\n\
 mixing_beta = 0.7\n\
 conv_thr = 1.0d-8\n\
/\n\
ATOMIC_SPECIES\n\
 Al 26.98 Al.blyp-hgh.UPF\n\
 Si 28.086 Si.blyp-hgh.UPF\n\
K_POINTS {automatic}\n\
 4 4 4 0 0 0\n\
ATOMIC_POSITIONS {alat}\n"

def z2symbol(z, table):
    for it in table:
        if it[0] == z:
            return it[1]

def isoEnergy(z, table):
    for it in table:
        if it[0] == z:
            return it[2]

def writeIN(types, cluster, table):
    f = open('tmp.in', 'w')
    f.write(DefaultIn)
    for i in range(len(types)):
        if types[i] != 0:
            x = cluster[i][0]/2.0 + 0.5
            y = cluster[i][1]/2.0 + 0.5
            z = cluster[i][2]/2.0 + 0.5
            line = ' %s %3.2f %3.2f %3.2f\n' % (z2symbol(types[i], table), x, y, z)
            f.write(line)
    f.close()

def readOUT(outfile):
    f = open(outfile, 'r')
    for line in f:
        if line.find('!    total energy              =') == 0:
            sp = line.split()
            f.close()
            return float(sp[4])
    f.close()
    return 0.0

def calcQE(kmc, types):
    table = [[13, 'Al', -3.86102800], [14, 'Si', -7.45092356]]
    writeIN(types, kmc.cluster, table)
    outfile = '%s.out' % (MPKMC.types2string(types))
    cmd = './pw.x < tmp.in > %s' % (outfile)
    print cmd
    subprocess.call(cmd, shell=True)
    tote = readOUT(outfile)
    totc = 0
    for tp in types:
        if tp != 0:
            tote = tote-isoEnergy(tp, table)
            totc = totc+1
    print types, totc, tote/totc
    return tote/totc

if __name__ == "__main__":
    Ntry = 100000
    T = range(30, 0, -2)
    etbfile = 'Al-Si.etb'
    resfile = 'Al-Si_1.mpkmc'
    Kb = 86.1735e-6; # ev/K
    Ry = 13.6058; # ev
    Kbry = Kb/Ry
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (13, 13, 13, 13)
    pv = ((4.08, 0, 0), (0, 4.08, 0), (0, 0, 4.08))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5)) 
    jclu = (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1)
    kmc = MPKMC.new(4, 10, 10, 10, 13, 100, 1000, 10000)
    kmc.rand_seed = 12345
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, jclu)
    kmc.calc_rot_index(5.0, 1.0e-6)
    if os.path.exists(etbfile):
        kmc.read_table(etbfile)
    kmc.add_solute_random(40, 14, 1)
    tote, update = kmc.total_energy(calcQE)
    if update:
        kmc.write_table(etbfile)
    print kmc.tote, kmc.ntable
    for temp in T:
        njump, update = kmc.jump(Ntry, Kbry*temp, calcQE)
        if update:
            kmc.write_table(etbfile)
        print temp, njump, kmc.tote, kmc.ntable
    kmc.write(resfile, 8)

