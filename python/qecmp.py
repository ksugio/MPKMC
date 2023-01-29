import MPKMC
import qetool
import tensorflow as tf
import numpy as np
import pandas as pd

MLModelFile = 'Al-Si-V.h5'
#Replace = {13: 0, 14: 1}
Replace = {0: 0, 13: 1, 14: 2}
MLModel = tf.keras.models.load_model(MLModelFile)

def calcBond(kmc, types):
    ebonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0 }
    return qetool.bondClusterEnergy(types, ebonds)

def calcML2(kmc, ntypes, types):
    retypes = []
    for i in range(ntypes):
        rtps = []
        for j in range(kmc.ncluster):
            tp = types[i * kmc.ncluster + j]
            rtps.append(Replace[tp])
        retypes.append(rtps)
    energy = MLModel.predict(retypes).reshape(ntypes)
    return tuple(energy)

def Compare(kmc, order, isolates):
    results = []
    for outfile, types in order:
        data = qetool.readQEout(outfile)
        tote = qetool.BindingEnergy(data, isolates)
        ctypes = qetool.changeType(kmc, data)
        kmc.grid_energy(calcBond)
        mcene = kmc.tote
        kmc.grid_energy2(calcML2)
        mlene = kmc.tote
        res = (outfile, tote, mcene, mlene)
        results.append(res)
        print(res, ctypes)
    return results

#
# compare QE with MC calculated by ML potential
#
if __name__ == "__main__":
    outdir = 'AlSi60'
    etbfile = 'Al-Si-V.etb'
    cmpfile = 'Al-Si-V-cmp.csv'
    isolates = {0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    kmc.read_table(etbfile)
    ctypes = qetool.countQEoutClusterTypes(outdir)
    results = []
    # Al cluster
    al = [('AlSi60/qAAAAAAAAAAAAA.out',  {13: 32})]
    results.extend(Compare(kmc, al, isolates))
    # Select Al-Si cluster
    alsi = [t for t in ctypes if not 0 in t[1] and 14 in t[1]]
    alsi.sort(key=lambda n: n[1][14])
    results.extend(Compare(kmc, alsi, isolates))
    # Select Al-V cluster
    alv = [t for t in ctypes if not 14 in t[1] and 0 in t[1]]
    alv.sort(key=lambda n: n[1][0])
    results.extend(Compare(kmc, alv, isolates))
    # Select Al-Si-V cluster
    alsiv = [t for t in ctypes if 13 in t[1] and 14 in t[1] and 0 in t[1]]
    alsiv.sort(key=lambda n: n[1][14])
    alsiv.sort(key=lambda n: n[1][0])
    results.extend(Compare(kmc, alsiv, isolates))
    # Write results
    df = pd.DataFrame(results, columns=['File', 'QE', 'MC', 'ML'])
    df['Error_MC_QE'] = np.fabs(df['MC'] - df['QE'])
    df['Error_ML_QE'] = np.fabs(df['ML'] - df['QE'])
    df['Error_ML_MC'] = np.fabs(df['ML'] - df['MC'])
    df.to_csv(cmpfile)
