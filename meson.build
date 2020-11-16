project('helen3d', 'cpp', 'c')
qt5 = import('qt5')

qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'Sql'], required : false)
dep_gl = dependency('gl', required : false)

boost_dep = dependency('boost')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

# Run the version and hash generator
hash_script = find_program('get_hash.sh')

# This will produce the dependency file.
run_command('get_hash.sh')

gen_src = custom_target('commit-output',
                        input : ['libsrc/commit.h.pre'],
                        output : ['commit.h'],
                        command : [hash_script, '@INPUT@', '@OUTPUT@'],
                        build_by_default: true,
                        build_always_stale: true)

moc_files = qt5.preprocess(moc_headers : 
['libsrc/SlipGL.h',
'libsrc/Mesh.h',
'libsrc/Plot3D.h',
'libsrc/CurveView.h',
'libsrc/Curve.h',
],
			moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])

helen3d = library('helen3d', gen_src, moc_files,
'libsrc/CurveView.cpp',
'libsrc/Curve.cpp',
'libsrc/Icosahedron.cpp',
'libsrc/FileReader.cpp',
#'libsrc/GLObject.cpp',
'libsrc/lbfgs.c',
'libsrc/Mesh.cpp',
'libsrc/Plot3D.cpp',
'libsrc/RefinementGridSearch.cpp', 
'libsrc/RefinementLBFGS.cpp', 
'libsrc/RefinementList.cpp', 
'libsrc/RefinementNelderMead.cpp', 
'libsrc/RefinementStepSearch.cpp', 
'libsrc/RefinementStrategy.cpp', 
'libsrc/shader.cpp',
'libsrc/SlipGL.cpp',
'libsrc/SlipObject.cpp',
'libsrc/SlipObjFile.cpp',
'libsrc/Timer.cpp', 
'libsrc/vec3.cpp',
'libsrc/mat3x3.cpp',
'libsrc/mat4x4.cpp',
cpp_args: ['-std=c++11'], dependencies : [ boost_dep, qt5_dep ], 
install: true)

install_headers([
'libsrc/Curve.h',
'libsrc/CurveView.h',
'libsrc/Dialogue.h',
'libsrc/FileReader.h',
'libsrc/Frameworks.h',
#'libsrc/GLObject.h',
'libsrc/Icosahedron.h',
'libsrc/Mesh.h',
'libsrc/Pic2Header.h',
'libsrc/Picture.h',
'libsrc/Plot3D.h',
'libsrc/RefinementGridSearch.h',
'libsrc/RefinementLBFGS.h',
'libsrc/RefinementList.h',
'libsrc/RefinementNelderMead.h',
'libsrc/RefinementStepSearch.h',
'libsrc/RefinementStrategy.h',
'libsrc/SlipGL.h',
'libsrc/SlipObjFile.h',
'libsrc/SlipObject.h',
'libsrc/Timer.h', 
'libsrc/charmanip.h',
'libsrc/mat3x3.h',
'libsrc/mat4x4.h',
'libsrc/vec3.h',
],
 subdir : 'helen3d')

pkg = import('pkgconfig')
h = ['helen3d'] # subdirectories of ${prefix}/${includedir} to add to header path
pkg.generate(libraries : [helen3d],
             subdirs : ['helen3d'],
             version : '1.0',
             name : 'helen3d',
             filebase : 'helen3d',
             description : 'Library for 3D utilities by Helen')

