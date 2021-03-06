#
# Meson project file for the ftinspect program.
#

# Copyright (C) 2021-2022 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.

qt5 = import('qt5')
qt5_dep = dependency('qt5',
  required: false,
  modules: ['Core', 'Gui', 'Widgets'])

# Don't compile `ftinspect` if Qt5 is not found.  This can happen
# when cross-compiling to a different system.
if qt5_dep.found()
  sources = files([
    'engine/engine.cpp',
    'engine/fontfilemanager.cpp',

    'rendering/glyphbitmap.cpp',
    'rendering/glyphoutline.cpp',
    'rendering/glyphpointnumbers.cpp',
    'rendering/glyphpoints.cpp',
    'rendering/glyphcontinuous.cpp',
    'rendering/grid.cpp',
    'rendering/graphicsdefault.cpp',
    'rendering/renderutils.cpp',
    'widgets/customwidgets.cpp',
    'widgets/glyphindexselector.cpp',
    'widgets/fontsizeselector.cpp',

    'models/ttsettingscomboboxmodel.cpp',

    'panels/settingpanel.cpp',
    'panels/singular.cpp',
    'panels/continuous.cpp',

    'ftinspect.cpp',
    'maingui.cpp',
    'uihelper.cpp',
])

  moc_files = qt5.preprocess(
    moc_headers: [
      'engine/fontfilemanager.hpp',
      'widgets/customwidgets.hpp',
      'widgets/glyphindexselector.hpp',
      'widgets/fontsizeselector.hpp',
      'widgets/glyphcontinuous.hpp',
      'models/ttsettingscomboboxmodel.hpp',
      'panels/settingpanel.hpp',
      'panels/singular.hpp',
      'panels/continuous.hpp',
      'maingui.hpp',
    ],
    dependencies: qt5_dep)

  executable('ftinspect',
    sources,
    moc_files,
    dependencies: [qt5_dep, libfreetype2_dep],
    override_options : ['cpp_std=c++11'],
    install: true)
endif

# EOF
