project(
	'sem3lab3',
	'cpp',
	version: '0.1',
	license: 'MIT',
	meson_version: '>=1.4',
	default_options: [
		'cpp_std=c++23',
		'warning_level=3',
		'werror=true',
	],
)

add_project_arguments(
	[
		'-D_POSIX_C_SOURCE=200809L',
		#'-DWLR_USE_UNSTABLE',
	],
	language: 'cpp',
)

cc = meson.get_compiler('cpp')

add_project_arguments(
	cc.get_supported_arguments(
		[
			'-Wundef',
			'-Wlogical-op',
			'-Wmissing-include-dirs',
			'-Wold-style-definition',
			'-Wpointer-arith',
			'-Winit-self',
			'-Wfloat-equal',
			'-Wstrict-prototypes',
			'-Wredundant-decls',
			'-Wimplicit-fallthrough',
			'-Wendif-labels',
			#'-Wstrict-aliasing=2',
			'-Woverflow',
			'-Wmissing-prototypes',
			'-Walloca',
			'-Wvla',
			'-Wformat=2',
			#'-Wold-style-cast',
			'-Wshadow',

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
)

#subdir('src')

sdl3_dep = dependency('sdl3')
sdl3_image_dep = dependency('sdl3_image', default_options: ['werror=false'])
sdl3_ttf_dep = dependency('sdl3_ttf', default_options: ['werror=false'])
imgui_dep = dependency('imgui')
#sdl2_mixer_dep = dependency('SDL2_mixer')
executable(
	'sem3lab3',
	src,
	include_directories: [include],
	dependencies: [sdl3_dep, sdl3_image_dep, sdl3_ttf_dep, imgui_dep],
	#link_with: [libs],
	#install: true
)
