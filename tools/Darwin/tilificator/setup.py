#!/usr/bin/env python

# Build c_tile extension
from distutils.core import setup, Extension

try:
    from versioning import VERSION_STRING
except:
    VERSION_STRING = 'unknown'

module1 = Extension('c_tile', sources=['c_tile.c'])
setup(name='c_tile',
      version='1.0',
      description='Library of optimized tile functions',
      ext_modules=[module1])
extra_compile_args = ["-O3"]  # You could put "-O4" etc. here.

# Install as library
# from setuptools import find_packages
# modules = ['array2d',
#            'common',
#            'import_raw_tiles',
#            'sprite',
#            'spritetile',
#            'tile',
#            'tilificator',
#            'TilificatorProject',
#            'tilingmethod',
#            'tilingmethoddragqueen',
#            'tilingmethodfullmatch',
#            'tilingmethodrect'
#            'tilingmethodshiftedrows']
# setup(name='tilificator', version='0.7.5', packages=[''])


