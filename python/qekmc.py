import MPKMC
import qetool
import subprocess
import tensorflow as tf
import os

Isolates = { 0: 0.0, 13: -4.76961123, 14: -11.02436232, }
EBonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
           (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
           (0, 14): -0.03314451484373948, (0, 0): 0.0 }

# calculate energy with starting pw.x
def calcQE(kmc, types):
    qedir = 'AlSiV'
    headfile = 'qehead.in'
    infile = '%s/%s.in' % (qedir, MPKMC.types2string(types))
    qetool.writeQEin(types, kmc.cluster, headfile, infile, fill_type=13)
    outfile = '%s/%s.out' % (qedir, MPKMC.types2string(types))
    if not os.path.exists(outfile):
        cmd = './pw.x < %s > %s' % (infile, outfile)
        print(cmd)
        subprocess.call(cmd, shell=True)
    data = qetool.readQEout(outfile)
    work = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    cid = work.grid2index(0, 1, 1, 1)
    tote = qetool.BindingEnergy(data, Isolates)
    qetool.changeType(work, data)
    clue, _, _ = qetool.ClusterEnergy2(kmc, work, tote, EBonds, cid)
    return clue

# calculate energy with bond energy
def calcBond(kmc, types):
    return qetool.bondClusterEnergy(types, EBonds)

# calculate energy with ML potential
MLModelFile = 'Al-Si-V.h5'
Replace = {0: 0, 13: 1, 14: 2}
MLModel = tf.keras.models.load_model(MLModelFile)
def calcML(kmc, types):
    retypes = []
    for i in range(kmc.ncluster):
        retypes.append(Replace[types[i]])
    clue = MLModel.predict([retypes], verbose=0)[0][0]
    return clue

if __name__ == "__main__":
    Ntry = 1000
    T = range(800, 300, -10)
    etbfile = 'Al-Si-V.etb'
    resfile = 'Al-Si.mpkmc'
    Ry = 13.6058
    calcfunc = calcBond
    kmc = qetool.FCCCell(13, 4.04360, 5, 5, 5)
    kmc.freq = 1.0e13 # frequency of atom
    kmc.set_migration(0, 13, 0.66 / Ry)
    kmc.set_migration(0, 14, 0.66 / Ry)
    kmc.set_migration(13, 14, 0.66 / Ry)
    kmc.rand_seed = 12345
    if os.path.exists(etbfile):
        kmc.read_table(etbfile)
    kmc.add_solute_random(25, 14, 1)
    #kmc.add_solute_random(10, 0, 1)
    #kmc.add_solute(28, 14, 1)
    #kmc.add_solute(9, 14, 1)
    #kmc.grid_energy(calcfunc)
    kmc.list_transition(calcfunc)
    print(kmc.tote, kmc.ntable)
    print('(totmcs, temp, ntry, njump, table_update, ntable, tote, tott, time) ngroup')
    for temp in T:
        ret = kmc.jump2(Ntry, temp, calcfunc)
        print(ret, kmc.find_solute_group(1.42))
    kmc.write(resfile, 8)
