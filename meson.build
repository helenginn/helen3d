project('helen3d', 'cpp', 'c')
qt5 = import('qt5')
cxx = meson.get_compiler('cpp')
arg_list = []

if (host_machine.system() == 'darwin')
  arg_list += '-mmacosx-version-min=10.14'
endif

qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'Sql'], required : false)
dep_gl = dependency('gl', required : false)

boost_dep = dependency('boost')

helen3d_scs = false

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

# Run the version and hash generator
hash_script = find_program('get_hash.sh')

# This will produce the dependency file.
run_command('get_hash.sh')

# this will obtain the helencore
sp_helencore = subproject('helencore')
helencore_dep = sp_helencore.get_variable('helencore_subdep')

if qt5_dep.found() and dep_gl.found()
helen3d_scs = true
moc_files = qt5.preprocess(moc_headers : 
['h3dsrc/SlipGL.h',
'h3dsrc/Mesh.h',
'h3dsrc/Plot3D.h',
'h3dsrc/CurveView.h',
'h3dsrc/Curve.h',
'h3dsrc/Dictator.h',
],
			moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])

#helencore_headers = sp_helencore.get_variable('helencore_inc')
helencore_headers = ['subprojects/helencore']

helen3d = library('helen3d', moc_files,
'h3dsrc/CurveView.cpp',
'h3dsrc/Curve.cpp',
'h3dsrc/Dictator.cpp',
'h3dsrc/Icosahedron.cpp',
'h3dsrc/lbfgs.c',
'h3dsrc/Mesh.cpp',
'h3dsrc/Plot3D.cpp',
'h3dsrc/Quad.cpp',
'h3dsrc/shader.cpp',
'h3dsrc/SlipGL.cpp',
'h3dsrc/SlipObject.cpp',
'h3dsrc/SlipObjFile.cpp',
'h3dsrc/Text.cpp',
cpp_args: arg_list,
install_rpath: '$ORIGIN/.',
include_directories : helencore_headers,
dependencies : [ helencore_dep, boost_dep, qt5_dep ], 
link_args: arg_list,
install: true)

install_headers([
'h3dsrc/Curve.h',
'h3dsrc/CurveView.h',
'h3dsrc/Dialogue.h',
'h3dsrc/Dictator.h',
'h3dsrc/Frameworks.h',
'h3dsrc/Icosahedron.h',
'h3dsrc/Mesh.h',
'h3dsrc/Pic2Header.h',
'h3dsrc/Picture.h',
'h3dsrc/Plot3D.h',
'h3dsrc/Quad.h',
'h3dsrc/SlipGL.h',
'h3dsrc/SlipObjFile.h',
'h3dsrc/SlipObject.h',
'h3dsrc/SlipObject.h',
'h3dsrc/Text.h',
],
 subdir : 'h3dsrc')

install_headers([
'h3dsrc/shaders/vStructure.h',
'h3dsrc/shaders/fStructure.h',
'h3dsrc/shaders/shShadow.h',
],
 subdir : 'h3dsrc/shaders')

pkg = import('pkgconfig')
h = ['helen3d'] # subdirectories of ${prefix}/${includedir} to add to header path
pkg.generate([helen3d],
             subdirs : [''],
             version : '1.0',
             name : 'helen3d',
             filebase : 'helen3d',
             description : 'Library for 3D utilities by Helen')

helen3d_inc = include_directories('h3dsrc')
helen3d_subdep = declare_dependency(link_with: helen3d, 
include_directories: helen3d_inc)

endif
