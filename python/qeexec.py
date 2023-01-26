import qetool
import subprocess
import os

if __name__ == "__main__":
    qedir = 'qeAlSi'
    for infile in qetool.listQEout(qedir, ext='.in'):
        s = os.path.splitext(os.path.basename(infile))[0]
        outfile = '%s/%s.out' % (qedir, s)
        if not os.path.exists(outfile):
            cmd = './pw.x < tmp.in > %s' % (outfile)
            print(cmd)
            subprocess.call(cmd, shell=True)