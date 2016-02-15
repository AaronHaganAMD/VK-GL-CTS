# -*- coding: utf-8 -*-

#-------------------------------------------------------------------------
# Vulkan CTS
# ----------
#
# Copyright (c) 2015 Google Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice(s) and this permission notice shall be
# included in all copies or substantial portions of the Materials.
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
#
#-------------------------------------------------------------------------

import os
import sys
import string
import argparse
import tempfile
import shutil
import fnmatch

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", "scripts"))

from build.common import *
from build.config import *
from build.build import *

class Module:
	def __init__ (self, name, dirName, binName):
		self.name		= name
		self.dirName	= dirName
		self.binName	= binName

VULKAN_MODULE		= Module("dEQP-VK", "../external/vulkancts/modules/vulkan", "deqp-vk")
DEFAULT_BUILD_DIR	= os.path.join(tempfile.gettempdir(), "spirv-binaries", "{targetName}-{buildType}")
DEFAULT_TARGET		= "null"
DEFAULT_DST_DIR		= os.path.join(DEQP_DIR, "external", "vulkancts", "data", "vulkan", "prebuilt")

def getBuildConfig (buildPathPtrn, targetName, buildType):
	buildPath = buildPathPtrn.format(
		targetName	= targetName,
		buildType	= buildType)

	return BuildConfig(buildPath, buildType, ["-DDEQP_TARGET=%s" % targetName])

def cleanDstDir (dstPath):
	binFiles = [f for f in os.listdir(dstPath) if os.path.isfile(os.path.join(dstPath, f)) and fnmatch.fnmatch(f, "*.spirv")]

	for binFile in binFiles:
		print "Removing %s" % os.path.join(dstPath, binFile)
		os.remove(os.path.join(dstPath, binFile))

def execBuildPrograms (buildCfg, generator, module, mode, dstPath):
	workDir = os.path.join(buildCfg.getBuildDir(), "modules", module.dirName)

	pushWorkingDir(workDir)

	try:
		binPath = generator.getBinaryPath(buildCfg.getBuildType(), os.path.join(".", "vk-build-programs"))
		execute([binPath, "--mode", mode, "--dst-path", dstPath])
	finally:
		popWorkingDir()

def parseArgs ():
	parser = argparse.ArgumentParser(description = "Build SPIR-V programs",
									 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument("-b",
						"--build-dir",
						dest="buildDir",
						default=DEFAULT_BUILD_DIR,
						help="Temporary build directory")
	parser.add_argument("-t",
						"--build-type",
						dest="buildType",
						default="Debug",
						help="Build type")
	parser.add_argument("-c",
						"--deqp-target",
						dest="targetName",
						default=DEFAULT_TARGET,
						help="dEQP build target")
	parser.add_argument("--mode",
						dest="mode",
						default="build",
						help="Build mode (build or verify)")
	parser.add_argument("-d",
						"--dst-path",
						dest="dstPath",
						default=DEFAULT_DST_DIR,
						help="Destination path")
	return parser.parse_args()

if __name__ == "__main__":
	args = parseArgs()

	generator	= ANY_GENERATOR
	buildCfg	= getBuildConfig(args.buildDir, args.targetName, args.buildType)
	module		= VULKAN_MODULE

	build(buildCfg, generator, ["vk-build-programs"])

	if args.mode == "build":
		if os.path.exists(args.dstPath):
			cleanDstDir(args.dstPath)
		else:
			os.makedirs(args.dstPath)

	execBuildPrograms(buildCfg, generator, module, args.mode, args.dstPath)
