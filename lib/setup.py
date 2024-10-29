from setuptools import setup, Extension
from glob import glob
import os
import numpy as np

if os.name == 'nt':
    setup(
        ext_modules = [
            Extension(
                name="MPKMC.MPKMC",
                sources=glob("*.c"),
                define_macros=[('MP_PYTHON_LIB', None)],
                include_dirs=['../zlib/include', np.get_include()],
                library_dirs=['../zlib/lib/x64'],
                libraries=['zlibstat']
            )
        ]
    )
else:
    setup(
        ext_modules = [
            Extension(
                name="MPKMC.MPKMC",
                sources=glob("*.c"),
                define_macros=[('MP_PYTHON_LIB', None)],
                libraries=['z']
            )
        ]
    )
