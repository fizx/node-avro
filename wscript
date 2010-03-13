import Options
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
  conf.env.append_value("LIB_AVRO",     "avro")
  conf.env.append_value("CPPPATH_AVRO", abspath("./deps/build/include/"))

  if os.system("cd \"deps/avro\" && ./configure --prefix=" + abspath("deps/build") + " && cd ..") != 0:
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
  node_avro.uselib = "AVRO"
  
def shutdown():
  # HACK to get binding.node out of build directory.
  # better way to do this?
  if Options.commands['clean']:
    if exists('lib/node-avro.node'): unlink('lib/node-avro.node')  
    rmtree('build')
    rmtree('deps/build')
    os.system("cd \"deps/avro\" && make distclean")
  else:
    if exists('build/default/node-avro.node') and not exists('node-avro.node'):
      copy('build/default/node-avro.node', 'lib/node-avro.node')
  

