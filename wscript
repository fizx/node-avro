import Options, glob
from os.path import join, dirname, abspath, exists
from shutil import copy, rmtree
from os import unlink
import sys, os


srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.tool_options("compiler_cc")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("compiler_cc")
  conf.check_tool("node_addon")

  conf.env.append_value("LIBPATH_JANSSON", abspath("./deps/avro/jansson/"))
  conf.env.append_value("CPPPATH_JANSSON", abspath("./deps/avro/jansson/src/"))

  conf.env.append_value("LIBPATH_AVRO", abspath("./deps/build/lib/"))
  conf.env.append_value("STATICLIB_AVRO",["avro"])
  conf.env.append_value("CPPPATH_AVRO", abspath("./deps/build/include/"))

  if os.system("cd \"deps/avro\" && ./configure --prefix=" + abspath("deps/build") + " && cd ..") != 0:
      conf.fatal("Configuring %s failed." % (subdir))  

  if os.system("cd \"deps/avro/jansson\" && ./configure --prefix=" + abspath("deps/build") + " && cd ..") != 0:
      conf.fatal("Configuring %s failed." % (subdir))  

def build(bld):

  # build avro
  os.system("cd \"deps/avro\" && make clean install")

  # build node-avro
  node_avro = bld.new_task_gen("cxx", "shlib", "node_addon")
  node_avro.source = bld.glob("binding.cc")
  node_avro.name = "node-avro"
  node_avro.target = "node-avro"
  node_avro.includes = "."
  node_avro.libpath = "lib"
  node_avro.uselib = "AVRO"

def shutdown():
  if exists('build/default/node-avro.node') and not exists('lib/node-avro.node'):
    copy('build/default/node-avro.node', 'lib/node-avro.node')
  
  if (exists('deps/build/lib/')):
    for FILE in glob.glob("deps/build/lib/*"):
      copy(FILE, "lib")
      

def clean(cln):
  if exists('build'): rmtree('build')
  if exists('deps/build'): rmtree('deps/build')
  if exists('deps/avro/Makefile'):
    os.system("cd \"deps/avro\" && make distclean")
  if exists('lib/node-avro.node'):
    for FILE in glob.glob("lib/*"):
      unlink(FILE)
    
  
def test(tst):
  print os.system("node test/sanity.js")
 

