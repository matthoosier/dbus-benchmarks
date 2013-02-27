#! /usr/bin/env python

from waflib import Configure

Configure.autoconfig = True

def options(opt):
	opt.load('compiler_c compiler_cxx')

def configure(conf):
	conf.load('compiler_c compiler_cxx')
	conf.check_cfg(package='dbus-1', args='--cflags --libs')

	if 'linux' == conf.env.DEST_OS:
		conf.env.append_value('LIB', 'pthread')
	elif 'qnx' == conf.env.DEST_OS:
		conf.env.append_value('LIB', 'socket')

def build(bld):
	bld.program(source='dbus-client.cc', uselib='DBUS-1', target='dbus-client')
	bld.program(source='dbus-server.cc', uselib='DBUS-1', target='dbus-server')
	bld.program(source='socket-client.cc', target='socket-client')
	bld.program(source='socket-server.cc', target='socket-server')
