import MPKMC
import qetool
import tensorflow as tf
import pandas as pd

MLModelFile = 'Al-Si.h5'
CMPFile = 'Al-Si-cmp.csv'
Replace = {13: 0, 14: 1}
#Replace = {0: 0, 13: 1, 14: 2}
MLModel = tf.keras.models.load_model(MLModelFile)

def calcQEwML(kmc, types):
    retypes = []
    for i in range(kmc.ncluster):
        retypes.append(Replace[types[i]])
    clue = MLModel.predict([retypes], verbose=0)[0][0]
    return clue

def Compare(kmc, order, isolates):
    results = []
    for outfile, types in order:
        data = qetool.readQEout(outfile)
        tote = qetool.BindingEnergy(data, isolates)
        ctypes = qetool.changeType(kmc, data)
        kmc.grid_energy(calcQEwML)
        diff = abs(kmc.tote - tote)
        res = (outfile, tote, kmc.tote, diff)
        results.append(res)
        print(res, ctypes)
    return results

#
# compare QE with MC calculated by ML potential
#
if __name__ == "__main__":
    outdir = 'AlSi60'
    isolates = {0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    kmc.table_use = False
    ctypes = qetool.countQEoutClusterTypes(outdir)
    results = []
    # Al cluster
    al = [('AlSi60/qAAAAAAAAAAAAA.out',  {13: 32})]
    results.extend(Compare(kmc, al, isolates))
    # Select Al-Si cluster
    alsi = [t for t in ctypes if not 0 in t[1] and 14 in t[1]]
    alsi.sort(key=lambda n: n[1][14])
    results.extend(Compare(kmc, alsi, isolates))
    # Write results
    df = pd.DataFrame(results, columns=['File', 'QE', 'MC', 'Error'])
    df.to_csv(CMPFile)
