import MPKMC
import math
import os
import itertools

Symbols = {1: 'H', 2: 'He', 3: 'Li', 4: 'Be', 5: 'B', 6: 'C',
           7: 'N', 8: 'O', 9: 'F', 10: 'Ne', 11: 'Na', 12: 'Mg',
           13: 'Al', 14: 'Si', 15: 'P', 16: 'S', 17: 'Cl', 18: 'Ar',
           19: 'K', 20: 'Ca', 21: 'Sc', 22: 'Ti', 23: 'V', 24: 'Cr',
           25: 'Mn', 26: 'Fe', 27: 'Co', 28: 'Ni', 29: 'Cu', 30: 'Zn',
           31: 'Ga', 32: 'Ge', 33: 'As', 34: 'Se', 35: 'Br', 36: 'Kr',
           37: 'Rb', 38: 'Sr', 39: 'Y', 40: 'Zr', 41: 'Nb', 42: 'Mo',
           43: 'Tc', 44: 'Ru', 45: 'Rh', 46: 'Pd', 47: 'Ag', 48: 'Cd'}

EPS = 1.0e-5

FCC222 = [(0.5, 0.0, 0.5), (0.75, 0.0, 0.75), (0.0, 0.5, 0.5), (0.0, 0.75, 0.75),
          (0.0, 0.0, 0.5), (0.25, 0.0, 0.75), (0.0, 0.25, 0.75), (0.5, 0.5, 0.0),
          (0.75, 0.75, 0.0), (0.5, 0.0, 0.0), (0.75, 0.25, 0.0), (0.75, 0.0, 0.25),
          (0.0, 0.5, 0.0), (0.25, 0.75, 0.0), (0.0, 0.75, 0.25), (0.0, 0.0, 0.0),
          (0.25, 0.25, 0.0), (0.25, 0.0, 0.25), (0.0, 0.25, 0.25), (0.5, 0.5, 0.5),
          (0.75, 0.75, 0.5), (0.5, 0.75, 0.25), (0.25, 0.5, 0.25), (0.25, 0.75, 0.5),
          (0.5, 0.75, 0.75), (0.75, 0.5, 0.75), (0.75, 0.25, 0.5), (0.5, 0.25, 0.75),
          (0.25, 0.5, 0.75), (0.25, 0.25, 0.5), (0.5, 0.25, 0.25), (0.75, 0.5, 0.25)]

def readQEout(outfile):
    znumbers = {v: k for k, v in Symbols.items()}
    data = {}
    count = -1
    lattice = []
    with open(outfile, 'r') as f:
        for line in f:
            if line.find('     number of atoms/cell      =') == 0:
                sp = line.split()
                data['NAtom'] = int(sp[4])
            elif line.find('     site n.     atom                  positions (alat units)') == 0:
                count += 1
            elif count >= 0 and count < data['NAtom']:
                sp = line.split()
                lattice.append((znumbers[sp[1]], float(sp[6]), float(sp[7]), float(sp[8])))
                count += 1
            elif line.find('!    total energy              =') == 0:
                sp = line.split()
                data['TotalEnergy'] = float(sp[4])
        data['Lattice'] = lattice
    return data

def listFiles(dir, ext='.out'):
    files = []
    for f in os.listdir(dir):
        path = os.path.join(dir, f)
        if os.path.isfile(path) and path.endswith(ext):
            files.append(path)
    files.sort()
    return files

def countQEoutTypes(outdir, ext='.out', fill_ref=FCC222, fill_type=0):
    outtypes = []
    for outfile in listFiles(outdir, ext):
        data = readQEout(outfile)
        lattice = fillLattice(data['Lattice'], fill_ref, fill_type)
        types = {}
        for n, x, y, z in lattice:
            if not n in types:
                types[n] = 1
            else:
                types[n] += 1
        outtypes.append((outfile, types))
    return outtypes

def countQEoutClusterTypes(outdir, types, ext='.out'):
    outtypes = []
    for outfile in listFiles(outdir, ext):
        s = os.path.splitext(os.path.basename(outfile))[0]
        tps = MPKMC.string2types(s)
        dtps = { t:0 for t in types }
        flag = True
        for tp in tps:
            if tp in dtps:
                dtps[tp] += 1
            else:
                flag = False
        if flag:
            outtypes.append((outfile, dtps))
    return outtypes

def addSolute(kmc, data, sjump=1, vjump=1):
    types = {}
    for i in range(kmc.ntot):
        x0, y0, z0 = kmc.index2pos(i)
        found = False
        for n, x, y, z in data['Lattice']:
            x *= kmc.size[0]
            y *= kmc.size[1]
            z *= kmc.size[2]
            if abs(x-x0) < EPS and abs(y-y0) < EPS and abs(z-z0) < EPS:
                kmc.add_solute(i, n, sjump)
                if not n in types:
                    types[n] = 1
                else:
                    types[n] += 1
                found = True
        if not found:
            kmc.add_solute(i, 0, vjump)
            if not 0 in types:
                types[0] = 1
            else:
                types[0] += 1
    return types

def checkBoundary(dx, dy, dz):
    if dx >= 0.5:
        dx -= 1.0
    elif dx < -0.5:
        dx += 1.0
    if dy >= 0.5:
        dy -= 1.0
    elif dy < -0.5:
        dy += 1.0
    if dz >= 0.5:
        dz -= 1.0
    elif dz < -0.5:
        dz += 1.0
    return dx, dy, dz

def searchPairs(lattice, rcut):
    pairs = []
    natom = len(lattice)
    for i in range(natom):
        n0, x0, y0, z0 = lattice[i]
        for j in range(i, natom):
            if i != j:
                n, x, y, z = lattice[j]
                dx, dy, dz = checkBoundary(x-x0, y-y0, z-z0)
                r = math.sqrt(dx*dx+dy*dy+dz*dz)
                if r < rcut:
                    pairs.append((i, j))
    return pairs

def bondKey(n0, n1):
    if n0 < n1:
        key = (n0, n1)
    else:
        key = (n1, n0)
    return key

def fillLattice(lattice, ref, type):
    addlatt = []
    for x0, y0, z0 in ref:
        found = False
        for n, x, y, z in lattice:
            if abs(x - x0) < EPS and abs(y - y0) < EPS and abs(z - z0) < EPS:
                found = True
        if not found:
            addlatt.append((type, x0, y0, z0))
    lattice.extend(addlatt)
    return lattice

def countBonds(data, fill_ref=FCC222, fill_type=0, rcut=0.36):
    bonds = {}
    lattice = fillLattice(data['Lattice'], fill_ref, fill_type)
    pairs = searchPairs(lattice, rcut)
    for i, j in pairs:
        key = bondKey(lattice[i][0], lattice[j][0])
        if not key in bonds:
            bonds[key] = 1
        else:
            bonds[key] += 1
    return bonds

def BindingEnergy(data, isolates):
    tote = data['TotalEnergy']
    for n, x, y, z in data['Lattice']:
        tote -= isolates[n]
    return tote

def removeAtom(lattice, pos):
    count = 0
    index = -1
    for n, x, y, z in lattice:
        if abs(x - pos[0]) < EPS and abs(y - pos[1]) < EPS and abs(z - pos[2]) < EPS:
            index = count
        count += 1
    newlatt = lattice[:]
    if index >= 0:
        newlatt.pop(index)
    return newlatt

def BondEnergy(outfile, combi, isolates, ebonds):
    if combi[0] > combi[1]:
        n0, n1 = combi[1], combi[0]
        combi = n0, n1
    data = readQEout(outfile)
    bonds = countBonds(data)
    tote = BindingEnergy(data, isolates)
    for bond in bonds:
        if bond[0] != combi[0] or bond[1] != combi[1]:
            tote -= bonds[bond] * ebonds[bond]
    ebonds[combi] = tote / bonds[combi]
    return bonds, ebonds

def ClusterEnergy(data, isolates, ebonds, fill_ref=FCC222, fill_type=0,
                  center=(0.5, 0.5, 0.5), rcut=0.36):
    tote = BindingEnergy(data, isolates)
    lattice = fillLattice(data['Lattice'], fill_ref, fill_type)
    lattice = removeAtom(lattice, center)
    pairs = searchPairs(lattice, rcut)
    for i, j in pairs:
        key = bondKey(lattice[i][0], lattice[j][0])
        tote -= ebonds[key]
    return tote / 2.0

def removeType(lattice, type):
    newlatt = []
    for n, x, y, z in lattice:
        if n != type:
            newlatt.append((n, x, y, z))
    return newlatt

def writeQEin(types, cluster, headfile, infile, fill_ref=FCC222, fill_type=0):
    lattice = []
    for i in range(len(types)):
        x = cluster[i][0] / 2.0 + 0.5
        y = cluster[i][1] / 2.0 + 0.5
        z = cluster[i][2] / 2.0 + 0.5
        lattice.append((types[i], x, y, z))
    if fill_type > 0:
        lattice = fillLattice(lattice, fill_ref, fill_type)
    lattice = removeType(lattice, 0)
    with open(headfile, mode='r') as fin:
        head = fin.read()
        head = head.replace('nat = 32', 'nat = %d' % (len(lattice)))
    with open(infile, mode='w') as fout:
        fout.write(head)
        for n, x, y, z in lattice:
            line = ' %s %3.2f %3.2f %3.2f\n' % (Symbols[n], x, y, z)
            fout.write(line)

def FCCCell(type, alat, nx, ny, nz,
            nsolute_step = 1000, ntable_step = 1000,
            nevent_step = 100000, nhistory_step = 100):
    Kb = 86.1735e-6 # ev/K
    Ry = 13.6058 # ev
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (type, type, type, type)
    pv = ((alat, 0, 0), (0, alat, 0), (0, 0, alat))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5), \
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0), \
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5))
    kmc = MPKMC.new(len(uc), nx, ny, nz, len(clu),
                    nsolute_step, ntable_step, nevent_step, nhistory_step)
    kmc.kb = Kb/Ry
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, len(clu))
    kmc.calc_rot_index(5.0, 1.0e-6)
    return kmc

def changeType(kmc, data):
    types = {}
    for i in range(kmc.ntot):
        x0, y0, z0 = kmc.index2pos(i)
        found = False
        for n, x, y, z in data['Lattice']:
            x *= kmc.size[0]
            y *= kmc.size[1]
            z *= kmc.size[2]
            if abs(x-x0) < EPS and abs(y-y0) < EPS and abs(z-z0) < EPS:
                kmc.change_type(i, n)
                if not n in types:
                    types[n] = 1
                else:
                    types[n] += 1
                found = True
        if not found:
            kmc.change_type(i, 0)
            if not 0 in types:
                types[0] = 1
            else:
                types[0] += 1
    return types

def bondClusterEnergy(types, ebonds):
    tote = 0.0
    for type in types[1:]:
        key = bondKey(types[0], type)
        tote += ebonds[key]
    return tote / 2.0

def dummyForFind(kmc, types):
    return 99

def findClusters(kmc, data, cid):
    changeType(kmc, data)
    kmc.reset_table()
    kmc.grid_energy(dummyForFind)
    ctypes = kmc.cluster_types(cid)
    ctid = kmc.search_cluster(ctypes)
    known = []
    unknown = []
    for i in range(kmc.ntable):
        item = kmc.table_item(i)
        if i == ctid:
            center = item
        elif item[2] > 0:
            if item[1] == 99:
                unknown.append(item)
            else:
                known.append(item)
    return center, known, unknown

def fillUnkownCluster(kmc, outfile, isolates, ebonds, cid):
    data = readQEout(outfile)
    bine = BindingEnergy(data, isolates)
    tote = bine
    center, known, unknown = findClusters(kmc, data, cid)
    for clu in known:
        tote -= clu[1] * clu[2]
    for clu in unknown:
        clue = bondClusterEnergy(clu[0], ebonds)
        tote -= clue * clu[2]
    cluec = tote / center[2]
    if center[1] == 99:
        add = kmc.change_cluster(center[0], cluec, center[2])
    else:
        add = -1
    return outfile, cluec, len(unknown), add, bine

def deleteUnkownClusters(kmc):
    delclus = []
    for i in range(kmc.ntable):
        item = kmc.table_item(i)
        if item[1] == 99:
            delclus.append(item)
    for types, ene, count in delclus[::-1]:
        kmc.delete_cluster(types)
    return delclus

def fillUnkownClusters(kmc, order, isolates, ebonds, cid):
    results = []
    for ord in order:
        ret = fillUnkownCluster(kmc, ord[0], isolates, ebonds, cid)
        results.append(ret)
        if ret[2] > 1:
            print('Unknown', ret[2], 'in', ret[0])
    return results

def ClusterEnergy2(kmc, work, tote, ebonds, cid):
    btypes = {}
    for i in range(work.ntot):
        if i != cid:
            types = work.cluster_types(i)
            tid = kmc.search_cluster(types)
            if tid >=0:
                tote -= kmc.table_item(tid)[1]
            else:
                tote -= bondClusterEnergy(types, ebonds)
                if not types in btypes:
                    btypes[types] = 1
                else:
                    btypes[types] += 1
    types = work.cluster_types(cid)
    return tote, types, btypes

# def fillUnkownClusters2(kmc, work, order, isolates, ebonds, cid, overlap=False):
#     results = []
#     for ord in order:
#         outfile = ord[0]
#         data = readQEout(outfile)
#         if 'TotalEnergy' in data:
#             tote = BindingEnergy(data, isolates)
#             changeType(work, data)
#             clue, types, btypes = ClusterEnergy2(kmc, work, tote, ebonds, cid)
#             if kmc.search_cluster(types) >= 0:
#                 if overlap:
#                     add = kmc.add_cluster(types, clue, 1)
#                 else:
#                     add = -1
#             else:
#                 add = kmc.add_cluster(types, clue, 1)
#             results.append((outfile, clue, len(btypes), add, tote))
#         else:
#             print('Read Error:',outfile)
#     return results

def dummyForBuild(kmc, types):
    return 99

def buildClusters(kmc, types, cid):
    t0 = kmc.grid_item(0)[0]
    types[t0] = kmc.ncluster - sum(types.values())
    types = list(types.items())
    types.sort(key=lambda n: n[1])
    base = types.pop()[0]
    solute = []
    for tp, n in types:
        for i in range(n):
            solute.append(tp)
    clus = []
    for combi in itertools.permutations(range(kmc.ncluster), len(solute)):
        clu = [base for i in range(kmc.ncluster)]
        for i in range(len(solute)):
            clu[combi[i]] = solute[i]
        clus.append(tuple(clu))
    clus = list(set(clus))
    ids = kmc.cluster_indexes(cid)
    for clu in clus:
        for i in range(len(ids)):
            kmc.change_type(ids[i], clu[i])
        kmc.grid_energy(dummyForBuild)

def MLData(kmc):
    x = []
    y = []
    for i in range(kmc.ntable):
        types, ene, _ = kmc.table_item(i)
        x.append(types)
        y.append(ene)
    types = set()
    for tps in x:
        types |= set(tps)
    types = sorted(list(types))
    replace = dict(zip(types, range(len(types))))
    rex = []
    for xi in x:
        rexi = []
        for j in range(kmc.ncluster):
            rexi.append(replace[xi[j]])
        rex.append(rexi)
    return rex, y, replace