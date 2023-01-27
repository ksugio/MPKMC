import qetool
import subprocess
import os
import shutil
import time

def copyOutFiles(srcdir, dstdir, offset=0):
    names = []
    for infile in qetool.listFiles(dstdir, ext='.in'):
        s = os.path.splitext(os.path.basename(infile))[0]
        outfile = '%s/%s.out' % (dstdir, s)
        if not os.path.exists(outfile):
            names.append(s)
    for outfile in qetool.listFiles(srcdir, ext='.out'):
        s = os.path.splitext(os.path.basename(outfile))[0]
        for name in names:
            if name[offset:] == s:
                dstfile = '%s/%s.out' % (dstdir, name)
                print('COPY', outfile, dstfile)
                shutil.copy(outfile, dstfile)

def execQE(qedir, max_procs, command='./pw.x'):
    iofiles = []
    for infile in qetool.listFiles(qedir, ext='.in'):
        s = os.path.splitext(os.path.basename(infile))[0]
        outfile = '%s/%s.out' % (qedir, s)
        if not os.path.exists(outfile):
            iofiles.append((infile, outfile))
    procs = []
    while len(iofiles) > 0:
        if len(procs) < max_procs:
            files = iofiles.pop(0)
            #cmd = 'ping 127.0.0.1 -n 5 > nul'
            cmd = '%s < %s > %s' % (command, files[0], files[1])
            print(files, cmd)
            proc = subprocess.Popen(cmd, shell=True)
            procs.append(proc)
        else:
            finish = []
            for proc in procs:
                if proc.poll() is None:
                    finish.append(False)
                else:
                    finish.append(True)
            for i in range(len(procs))[::-1]:
                if finish[i]:
                    procs.pop(i)
            time.sleep(5)

if __name__ == "__main__":
    #srcdir = 'AlSi60'
    #dstdir = 'qeAlSi'
    #copyOutFiles(srcdir, dstdir, 5)
    qedir = 'qeAlSi'
    max_procs = 6
    execQE(qedir, max_procs)

