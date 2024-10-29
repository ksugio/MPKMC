from setuptools import setup, Extension
from glob import glob
import os
import numpy as np
import MPKMC

if os.name == 'nt':
    setup(
        ext_modules=[
            Extension(
                name='MPKMC.MPGLKMC',
                sources=glob("*.c"),
                include_dirs=['../lib', np.get_include()],
                define_macros=[('MP_PYTHON_LIB', None), ('_WINDOWS', None)],
                libraries=['opengl32', 'glu32']
            )
        ]
    )
else:
    setup(
        ext_modules=[
            Extension(
                name='MPKMC.MPGLKMC',
                sources=glob("*.c"),
                include_dirs=['../lib', np.get_include()],
                define_macros=[('MP_PYTHON_LIB', None)],
                libraries=['GL', 'GLU']
            )
        ]
    )
