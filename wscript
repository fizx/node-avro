import Options, glob
from os.path import join, dirname, abspath, exists
from shutil import copy, rmtree
from os import unlink, mkdir
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

  conf.env.append_value("LIBPATH_AVRO", abspath("./build/default/lib/"))
  conf.env.append_value("STATICLIB_AVRO",["avro"])
  conf.env.append_value("CPPPATH_AVRO", abspath("./build/default/include/"))
  
  conf.env.append_value("LIBPATH_JANSSON", abspath("./build/default/lib/"))
  conf.env.append_value("STATICLIB_JANSSON",["jansson"])

  # configure avro on x86_64
  fpic = ""
  if (conf.env['DEST_CPU'] == 'x86_64'):
    fpic="--with-pic"
  
  buildpath = abspath("build/default")
  cmd = "cd \"deps/avro\" && autoreconf -f -i && ./configure %s --prefix=%s"
  if os.system(cmd % (fpic,buildpath)) != 0:
    conf.fatal("Configuring avro failed.")  

  cmd = "cd \"deps/avro/jansson\" && ./configure %s --enable-static --disable-shared --prefix=%s"
  if os.system(cmd % (fpic,buildpath)) != 0:
     conf.fatal("Configuring jansson failed.")
  

def build(bld):

  # build avro
  os.system("cd \"deps/avro\" && make && make install")
  copy('deps/avro/jansson/src/.libs/libjansson.a', 'build/default/lib/libjansson.a')
  
  bld.add_group();
  # build node-avro
  node_avro = bld.new_task_gen("cxx", "shlib", "node_addon")
  node_avro.source = bld.glob("src/*.cc")
  node_avro.name = "node-avro-addon"
  node_avro.target = "node-avro-addon"
  node_avro.includes = [".", abspath("build/default/include/")]
  node_avro.uselib = ["AVRO", "JANSSON"]
  bld.add_post_fun(copynode)

def copynode(ctx):
  if not exists('lib'):
    mkdir('lib')
  if exists('build/default/node-avro-addon.node'):
    copy('build/default/node-avro-addon.node', 'lib/node-avro-addon.node')

def shutdown():
  copynode(None)

def clean(cln):
  if exists('build'): rmtree('build')
  if exists('.lock-wscript'): unlink('.lock-wscript')
  if exists('deps/avro/m4/'): rmtree('deps/avro/m4/')
  if exists('deps/avro/Makefile'):
    os.system("cd \"deps/avro\" && make distclean")
  if exists('lib/node-avro-addon.node'): unlink('lib/node-avro-addon.node')
  
def test(tst):
  os.system("node test/sanity.js")
 
