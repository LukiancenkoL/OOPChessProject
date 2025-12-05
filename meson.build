project(
	'chess',
	'cpp',
	version: '0.1',
	license: 'MIT',
	meson_version: '>=1.7',
	default_options: [
		'cpp_std=c++23',
		'warning_level=3',
		'werror=true',
	],
)

add_project_arguments(
	[
		'-D_POSIX_C_SOURCE=200809L',
		# wayland
		#'-DWLR_USE_UNSTABLE',
		'-D_XOPEN_SOURCE=700',
		# random, srandom
		'-D_DEFAULT_SOURCE',
	],
	language: 'cpp',
)

if get_option('optimization') != '0'
	add_project_arguments(
		[
			# requires compiling with optimization
			#'-D_FORTIFY_SOURCE=3',
			'-fhardened',
		],
		language: 'cpp',
	)
endif

cc = meson.get_compiler('cpp')
add_project_arguments(
	cc.get_supported_arguments(
		[
			'-Wundef',
			'-Wlogical-op',
			'-Wpointer-arith',
			'-Winit-self',
			'-Wfloat-equal',
			'-Wredundant-decls',
			'-Wimplicit-fallthrough',
			'-Wendif-labels',
			'-Woverflow',
			'-Walloca',
			'-Wvla',
			'-Wformat=2',
			#'-Wmissing-include-dirs',

			#'-Wold-style-cast',
			'-Wshadow',

			'-Wduplicated-cond',
			#'-Wconversion',
			'-Wswitch-default',

			#'-fanalyzer',
			#'-fno-math-errno',
			#'-fno-signed-zeros',
			#'-fno-trapping-math',
		],
	),
	language: 'cpp',
)

#datadir = get_option('datadir')
#sysconfdir = get_option('sysconfdir')
#prefix = get_option('prefix')

#math = cc.find_library('m', required: false)

include = include_directories('include')

src = files(
	'src/main.cpp',
	'src/app.cpp',
	'src/pieces.cpp',
	'src/stockfish.cpp', 
)

#subdir('src')

sdl3 = dependency('sdl3', default_options: ['werror=false'])
#sdl3i_obj = subproject('sdl3_image')
#sdl3i_dep = sdl3i_obj.get_variable(var_name)
sdl3_image = dependency('sdl3_image', default_options: ['werror=false'])
imgui = dependency('imgui')

#my_lib = cc.find_library('libimgui', dirs: ['/home/misha/personal/chess-sdl3/subprojects/imgui-1.91.6/build/'])

executable(
	'chess',
	src,
	include_directories: [include],
	dependencies: [sdl3, sdl3_image, imgui],
	#link_with: [my_lib],
	#install: true
)
