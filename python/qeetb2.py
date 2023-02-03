import MPKMC
import qetool
import pandas as pd

Isolates = { 0: 0.0, 13: -4.76961123, 14: -11.02436232, }
EBonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
           (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
           (0, 14): -0.03314451484373948, (0, 0): 0.0 }

def dummyQE(kmc, types):
    return qetool.bondClusterEnergy(types, EBonds)

def Compare(kmc, results):
    results2 = []
    kmc.table_add = False
    for outfile, clue, unknown, add, tote in results:
        data = qetool.readQEout(outfile)
        qetool.changeType(kmc, data)
        kmc.grid_energy(dummyQE)
        diff = abs(kmc.tote-tote)
        results2.append((outfile, clue, unknown, add, tote, kmc.tote, diff))
    kmc.sort_table()
    return results2

def WriteResults(results, resfile):
    df = pd.DataFrame(results, columns=['File', 'Energy', 'Unknown', 'Add', 'QE', 'MC', 'Error'])
    df.to_csv(resfile)
    print('Number of Results:', len(df))
    print('Maximum Error:', df['Error'].max())
    print('Mean Error:', df['Error'].mean())
    print('STD of Error:', df['Error'].std())

#
# genarate .etb file from QE .out files with cluster energy
# if the cluster dose not exsists in table, bond energy is utilized
#
if __name__ == "__main__":
    qedir = 'AlSi60'
    etbfile = 'Al-Si-V.etb'
    resfile = 'Al-Si-V-res.csv'
    headfile = 'qehead.in'
    types = [13, 14, 0]
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    cid = kmc.grid2index(0, 1, 1, 1)
    outtypes = qetool.countQEoutClusterTypes(qedir, types)
    for tp in types[1:]:
        outtypes.sort(key=lambda t: t[1][tp])
    results = qetool.fillUnkownClusters(kmc, outtypes, Isolates, EBonds, cid)
    delclus = qetool.deleteUnkownClusters(kmc)
    print('Unknown Cluster:', len(delclus))
    for clu, ene, count in delclus:
        infile = '%s/%s.in' % (qedir, MPKMC.types2string(clu))
        qetool.writeQEin(clu, kmc.cluster, headfile, infile, fill_type=13)
    results = Compare(kmc, results)
    WriteResults(results, resfile)
    kmc.write_table(etbfile)


