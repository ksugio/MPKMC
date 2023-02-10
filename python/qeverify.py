import qetool
import os
import pandas as pd

def calcBond(kmc, types):
    ebonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0 }
    print('Bond energy used!')
    return qetool.bondClusterEnergy(types, ebonds)

if __name__ == "__main__":
    qedir = 'qeAlSi'
    etbfile = 'Al-Si.etb'
    files = [('AlSi555_0', 0, 14), ('AlSi555_10', 10, 14), ('AlSi555_20', 20, 14),
             ('AlSi555_30', 30, 14), ('AlSi555_40', 40, 14), ('AlSi555_50', 50, 14)]
    isolates = {0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    kmctote = []
    names = []
    for name, num, type in files:
        kmc = qetool.FCCCell(13, 4.04360, 5, 5, 5)
        kmc.rand_seed = 12345
        if num > 0:
            kmc.add_solute_random(num, type, 1)
        kmc.read_table(etbfile)
        kmc.grid_energy(calcBond)
        names.append(name)
        kmctote.append(kmc.tote)
        celldm = 15.2826 / 2 * 5
        infile = '%s/%s.in' % (qedir, name)
        if not os.path.exists(infile):
            qetool.writeQEinKMC(kmc, 'qehead.in', infile, celldm)
    qetote = []
    for name, num, type in files:
        outfile = '%s/%s.out' % (qedir, name)
        if os.path.exists(outfile):
            data = qetool.readQEout(outfile)
            tote = qetool.BindingEnergy(data, isolates)
            qetote.append(tote)
        else:
            qetote.append(None)
    df = pd.DataFrame([names, qetote, kmctote]).T
    df.columns = ['Name', 'QE', 'MC']
    print(df)

