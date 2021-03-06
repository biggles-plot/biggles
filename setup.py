#!/usr/bin/env python
#
# $Id: setup.py,v 1.34 2010/04/09 21:28:15 mrnolta Exp $
#
# Copyright (C) 2001-10 :
#
#       Berthold Hollmann <bhoel@starship.python.net>
#       Mike Nolta <mike@nolta.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA  02111-1307, USA.
#

#
# distutils setup file for biggles originally contributed
# by Berthold Hollmann.
#

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import os
from subprocess import Popen
import glob
import shutil


class build_ext_subclass(build_ext):
    plotutils_dir = 'plotutils'

    def initialize_options(self):
        build_ext.initialize_options(self)

    def finalize_options(self):
        build_ext.finalize_options(self)
        self.plotutils_build_dir = os.path.join(
            self.build_temp,
            self.plotutils_dir,
        )
        # self.plot_h_dir = os.path.join(self.plotutils_dir, 'include')
        # self.libplot_dir = os.path.join(
        #     self.plotutils_dir, 'libplot', '.libs',
        # )
        self.plot_h_dir = os.path.join(
            self.plotutils_build_dir, 'include')
        self.libplot_dir = os.path.join(
            self.plotutils_build_dir, 'libplot', '.libs',
        )

        self.include_dirs.insert(0, self.plot_h_dir)

    def run(self):
        # For extensions that require 'numpy' in their include dirs,
        # replace 'numpy' with the actual paths
        import numpy
        np_include = numpy.get_include()

        for extension in self.extensions:
            if 'numpy' in extension.include_dirs:
                idx = extension.include_dirs.index('numpy')
                extension.include_dirs.insert(idx, np_include)
                extension.include_dirs.remove('numpy')

        build_ext.run(self)

    def build_extensions(self):

        # Use the compiler for building python to build plotutils
        # for maximized compatibility.

        CCold = self.compiler.compiler
        CC = []
        for val in CCold:
            """
            if val == '-O3':
                print("replacing '-O3' with '-O2' to address "
                      "gcc bug")
                val = '-O2'
            """
            if val == 'ccache':
                print("removing ccache from the compiler options")
                continue

            CC.append(val)

        self.configure_plotutils(
            CC=CC,
            ARCHIVE=self.compiler.archiver,
            RANLIB=self.compiler.ranlib,
        )

        libs = [
            # "plot",
            "Xaw", "Xmu", "Xt", "SM", "ICE", "Xext", "X11",
            "png", "z", "m",
        ]
        for lib in libs:
            self.compiler.add_library(lib)

        self.compile_plotutils()

        link_objects = glob.glob(
            os.path.join(self.libplot_dir, '*.a'),
        )

        self.compiler.set_link_objects(link_objects)

        # Ultimate hack: append the .a files to the dependency list
        # so they will be properly rebuild if source updated.
        for ext in self.extensions:
            ext.depends += link_objects

        # call the original build_extensions
        build_ext.build_extensions(self)

    def configure_plotutils(self, CC=None, ARCHIVE=None, RANLIB=None):

        # prepare source code and run configure
        def copy_update(dir1, dir2):
            f1 = os.listdir(dir1)
            for f in f1:
                path1 = os.path.join(dir1, f)
                path2 = os.path.join(dir2, f)

                if os.path.isdir(path1):
                    if not os.path.exists(path2):
                        os.makedirs(path2)
                    copy_update(path1, path2)
                else:
                    if not os.path.exists(path2):
                        shutil.copy(path1, path2)
                    else:
                        stat1 = os.stat(path1)
                        stat2 = os.stat(path2)
                        if (stat1.st_mtime > stat2.st_mtime):
                            shutil.copy(path1, path2)

        if not os.path.exists('build'):
            os.makedirs('build')

        if not os.path.exists(self.plotutils_build_dir):
            os.makedirs(self.plotutils_build_dir)

        copy_update(self.plotutils_dir, self.plotutils_build_dir)

        makefile = os.path.join(self.plotutils_build_dir, 'Makefile')

        if os.path.exists(makefile):
            # Makefile already there
            return

        p = Popen(
            "sh ./configure --enable-shared=no --with-pic",
            shell=True,
            cwd=self.plotutils_build_dir,
        )
        p.wait()
        if p.returncode != 0:
            raise ValueError("could not configure plotutils")

    def compile_plotutils(self):
        p = Popen(
            "make",
            shell=True,
            cwd=self.plotutils_build_dir,
        )
        p.wait()
        if p.returncode != 0:
            raise ValueError("could not compile plotutils")


fp = open('README.rst', 'r')
long_description = fp.read()
fp.close()

biggles_ext = Extension(
    "_biggles",
    ["biggles/_biggles.c"],
    include_dirs=['numpy'],
    library_dirs=['/usr/X11/lib'],
)
biggles_libplot_ext = Extension(
    "libplot._libplot_pywrap",
    ["biggles/libplot/_libplot_pywrap.c"],
    include_dirs=['numpy'],
    library_dirs=['/usr/X11/lib'],
)
ext_modules = [biggles_ext, biggles_libplot_ext]

classifiers = [
    "Development Status :: 5 - Production/Stable",
    "License :: OSI Approved :: GNU General Public License (GPL)",
    "Topic :: Scientific/Engineering :: Astronomy",
    "Intended Audience :: Science/Research",
]

setup(
    name="biggles",
    version="2.0.0",
    author="Mike Nolta",
    author_email="mike@nolta.net",
    url="https://github.com/biggles-plot/biggles",
    license="GPL-2",
    description="simple, elegant python plotting",
    long_description=long_description,
    packages=["biggles", "biggles.libplot"],
    classifiers=classifiers,
    setup_requires=['numpy'],
    install_requires=['numpy'],
    use_2to3=True,
    ext_package="biggles",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext_subclass}
)
