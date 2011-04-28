srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

from os.path import exists 
from os import unlink, symlink

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'lzo'
  obj.source = 'src/minilzo/minilzo.cc src/lzo.cc'
  
def shutdown():
	if exists('build/default/lzo.node') and not exists('lzo.node'):
		symlink('build/default/lzo.node', 'lzo.node')
