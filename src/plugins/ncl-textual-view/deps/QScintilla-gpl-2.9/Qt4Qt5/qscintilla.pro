# The project file for the QScintilla library.
#
# Copyright (c) 2016 Riverbank Computing Limited <info@riverbankcomputing.com>
# 
# This file is part of QScintilla.
# 
# This file may be used under the terms of the GNU General Public License
# version 3.0 as published by the Free Software Foundation and appearing in
# the file LICENSE included in the packaging of this file.  Please review the
# following information to ensure the GNU General Public License version 3.0
# requirements will be met: http://www.gnu.org/copyleft/gpl.html.
# 
# If you do not wish to use this file under the terms of the GPL version 3.0
# then you may purchase a commercial license.  For more information contact
# info@riverbankcomputing.com.
# 
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


# This must be kept in sync with Python/configure.py, Python/configure-old.py,
# example-Qt4Qt5/application.pro and designer-Qt4Qt5/designer.pro.
!win32:VERSION = 12.0.2

TEMPLATE = lib
TARGET = qscintilla2_telem
CONFIG += qt warn_off release thread exceptions hide_symbols silent
INCLUDEPATH += . ../include ../lexlib ../src

!CONFIG(staticlib) {
    DEFINES += QSCINTILLA_MAKE_DLL
}


# Uses FORCERELEASE variable because CONFIG and SUBDIR force three executions
# if qmake and the last one does not preserves CONFIG from command line.
contains(FORCERELEASE, true) {
  CONFIG += qt warn_on release
  CONFIG -= debug
  DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_DEBUG_WARNING
  message ("qscintilla.pro RELEASE build!")
}
else {
  CONFIG += qt warn_on debug console
  CONFIG -= release
  message ("qscintilla.pro DEBUG build!")
}

# We use QMAKE_CXXFLAGS instead of INCLUDEPATH because our qscintilla is
# modified, and must be found before any other that is installed.
QMAKE_CXXFLAGS  += -I. -I../include -I../lexlib -I../src
DEFINES = QSCINTILLA_MAKE_DLL SCINTILLA_QT SCI_LEXER

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets printsupport

    greaterThan(QT_MINOR_VERSION, 1) {
	    macx:QT += macextras
    }

    # Work around QTBUG-39300.
    CONFIG -= android_install
}

# Comment this in if you want the internal Scintilla classes to be placed in a
# Scintilla namespace rather than pollute the global namespace.
#DEFINES += SCI_NAMESPACE

mac {
  QSCI_INSTALL_LIBS = $$quote(/Library/Application Support/Composer/Extensions)
  QSCI_INSTALL_HEADERS = $$[QT_INSTALL_HEADERS]
  QSCI_INSTALL_TRANSLATIONS = $$[QT_INSTALL_HEADERS]
  QSCI_INSTALL_DATA = $$[QT_INSTALL_DATA]

  QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -std=c++11 -stdlib=libc++
  LIBS += -mmacosx-version-min=10.7 -stdlib=libc++

  QMAKE_LFLAGS += -Wl,-install_name,'\'$$QSCI_INSTALL_LIBS/lib'$$TARGET'.dylib\''
}
else:unix {
  isEmpty(PREFIX) {
    PREFIX = /usr/local
  }

  QSCI_INSTALL_LIBS = $$PREFIX/lib/composer/plugins
  QSCI_INSTALL_HEADERS = $$PREFIX/include/composer
  QSCI_INSTALL_TRANSLATIONS = $$PREFIX/lib/composer/translations
  QSCI_INSTALL_DATA = $$PREFIX/lib/composer/
}
else:win32 {
  isEmpty(PREFIX) {
    PREFIX = "C:/Composer"
  }

  #TODO: This should be $$PREFIX/extensions, but on Windows we cannot link
  # against a lib inside the $$PREFIX/extensions path.
  QSCI_INSTALL_LIBS = $$PREFIX
  QSCI_INSTALL_HEADERS = $$PREFIX/include/composer
  #QSCI_INSTALL_TRANSLATIONS = $$PREFIX/translations
  QSCI_INSTALL_TRANSLATIONS = $$PREFIX/plugins
  QSCI_INSTALL_DATA = $$PREFIX/plugins
}

#QSCI_INSTALL_LIBS = $$[QT_INSTALL_LIBS]
#QSCI_INSTALL_HEADERS = $$[QT_INSTALL_HEADERS]
#QSCI_INSTALL_TRANSLATIONS = $$[QT_INSTALL_HEADERS]
#QSCI_INSTALL_DATA = $$[QT_INSTALL_DATA]

# Handle both Qt v4 and v3.
target.path = $$QSCI_INSTALL_LIBS
isEmpty(target.path) {
	target.path = $(QTDIR)/lib
}

header.path = $$QSCI_INSTALL_HEADERS
header.files = Qsci
isEmpty(header.path) {
	header.path = $(QTDIR)/include/Qsci
	header.files = Qsci/qsci*.h
}

trans.path = $$QSCI_INSTALL_TRANSLATIONS
trans.files = qscintilla_*.qm
isEmpty(trans.path) {
	trans.path = $(QTDIR)/translations
}

qsci.path = $$QSCI_INSTALL_DATA
qsci.files = ../qsci
isEmpty(qsci.path) {
	qsci.path = $(QTDIR)
}

INSTALLS += header trans qsci target

greaterThan(QT_MAJOR_VERSION, 4) {
    features.path = $$[QT_HOST_DATA]/mkspecs/features
} else {
    features.path = $$[QT_INSTALL_DATA]/mkspecs/features
}
CONFIG(staticlib) {
    features.files = $$PWD/features_staticlib/qscintilla2.prf
} else {
    features.files = $$PWD/features/qscintilla2.prf
}
INSTALLS += features

HEADERS = \
	./Qsci/qsciglobal.h \
	./Qsci/qsciscintilla.h \
	./Qsci/qsciscintillabase.h \
	./Qsci/qsciabstractapis.h \
	./Qsci/qsciapis.h \
	./Qsci/qscicommand.h \
	./Qsci/qscicommandset.h \
	./Qsci/qscidocument.h \
	./Qsci/qscilexer.h

contains(ALL_LEXERS, true) {
	HEADERS += \
	./Qsci/qscilexeravs.h \
	./Qsci/qscilexerbash.h \
	./Qsci/qscilexerbatch.h \
	./Qsci/qscilexercmake.h \
	./Qsci/qscilexercoffeescript.h \
	./Qsci/qscilexercpp.h \
	./Qsci/qscilexercsharp.h \
	./Qsci/qscilexercss.h \
	./Qsci/qscilexercustom.h \
	./Qsci/qscilexerd.h \
	./Qsci/qscilexerdiff.h \
	./Qsci/qscilexerfortran.h \
	./Qsci/qscilexerfortran77.h \
	./Qsci/qscilexerhtml.h \
	./Qsci/qscilexeridl.h \
	./Qsci/qscilexerjava.h \
	./Qsci/qscilexerjavascript.h \
	./Qsci/qscilexerlua.h \
	./Qsci/qscilexermakefile.h \
	./Qsci/qscilexermatlab.h \
	./Qsci/qscilexeroctave.h \
	./Qsci/qscilexerpascal.h \
	./Qsci/qscilexerperl.h \
	./Qsci/qscilexerpostscript.h \
	./Qsci/qscilexerpo.h \
	./Qsci/qscilexerpov.h \
	./Qsci/qscilexerproperties.h \
	./Qsci/qscilexerpython.h \
	./Qsci/qscilexerruby.h \
	./Qsci/qscilexerspice.h \
	./Qsci/qscilexersql.h \
	./Qsci/qscilexertcl.h \
	./Qsci/qscilexertex.h \
	./Qsci/qscilexerverilog.h \
	./Qsci/qscilexervhdl.h \
	./Qsci/qscilexerxml.h \
	./Qsci/qscilexeryaml.h
}
else {
	HEADERS += \
		./Qsci/qscilexercustom.h \
		./Qsci/qscilexercpp.h \
		./Qsci/qscilexerpython.h \
		./Qsci/qscilexerjavascript.h \
		./Qsci/qscilexerhtml.h \
                ./Qsci/qscilexerxml.h \
                ./Qsci/qscilexerlua.h \
}

HEADERS += \
	./Qsci/qscimacro.h \
	./Qsci/qsciprinter.h \
	./Qsci/qscistyle.h \
	./Qsci/qscistyledtext.h \
	ListBoxQt.h \
	SciClasses.h \
	SciNamespace.h \
	ScintillaQt.h \
	../include/ILexer.h \
	../include/Platform.h \
	../include/SciLexer.h \
	../include/Scintilla.h \
	../include/ScintillaWidget.h \
	../lexlib/Accessor.h \
	../lexlib/CharacterCategory.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexAccessor.h \
	../lexlib/LexerBase.h \
	../lexlib/LexerModule.h \
	../lexlib/LexerNoExceptions.h \
	../lexlib/LexerSimple.h \
	../lexlib/OptionSet.h \
	../lexlib/PropSetSimple.h \
	../lexlib/StringCopy.h \
	../lexlib/StyleContext.h \
	../lexlib/SubStyles.h \
	../lexlib/WordList.h \
	../src/AutoComplete.h \
	../src/CallTip.h \
	../src/CaseConvert.h \
	../src/CaseFolder.h \
	../src/Catalogue.h \
	../src/CellBuffer.h \
	../src/CharClassify.h \
	../src/ContractionState.h \
	../src/Decoration.h \
	../src/Document.h \
	../src/EditModel.h \
	../src/Editor.h \
	../src/EditView.h \
	../src/ExternalLexer.h \
	../src/FontQuality.h \
	../src/Indicator.h \
	../src/KeyMap.h \
	../src/LineMarker.h \
	../src/MarginView.h \
	../src/Partitioning.h \
	../src/PerLine.h \
	../src/PositionCache.h \
	../src/RESearch.h \
	../src/RunStyles.h \
	../src/ScintillaBase.h \
	../src/Selection.h \
	../src/SplitVector.h \
	../src/Style.h \
	../src/UnicodeFromUTF8.h \
	../src/UniConversion.h \
	../src/ViewStyle.h \
	../src/XPM.h

SOURCES = \
	qsciscintilla.cpp \
	qsciscintillabase.cpp \
	qsciabstractapis.cpp \
	qsciapis.cpp \
	qscicommand.cpp \
	qscicommandset.cpp \
	qscidocument.cpp \
	qscilexer.cpp

contains (ALL_LEXERS, true) {
	SOURCES += \
	qscilexeravs.cpp \
	qscilexerbash.cpp \
	qscilexerbatch.cpp \
	qscilexercmake.cpp \
	qscilexercoffeescript.cpp \
	qscilexercpp.cpp \
	qscilexercsharp.cpp \
	qscilexercss.cpp \
	qscilexercustom.cpp \
	qscilexerd.cpp \
	qscilexerdiff.cpp \
	qscilexerfortran.cpp \
	qscilexerfortran77.cpp \
	qscilexerhtml.cpp \
	qscilexeridl.cpp \
	qscilexerjava.cpp \
	qscilexerjavascript.cpp \
	qscilexerlua.cpp \
	qscilexermakefile.cpp \
	qscilexermatlab.cpp \
	qscilexeroctave.cpp \
	qscilexerpascal.cpp \
	qscilexerperl.cpp \
	qscilexerpostscript.cpp \
	qscilexerpo.cpp \
	qscilexerpov.cpp \
	qscilexerproperties.cpp \
	qscilexerpython.cpp \
	qscilexerruby.cpp \
	qscilexerspice.cpp \
	qscilexersql.cpp \
	qscilexertcl.cpp \
	qscilexertex.cpp \
	qscilexerverilog.cpp \
	qscilexervhdl.cpp \
	qscilexerxml.cpp \
	qscilexeryaml.cpp
}
else {
	SOURCES += \
		qscilexercustom.cpp \
		qscilexercpp.cpp \
		qscilexerpython.cpp \
		qscilexerjavascript.cpp \
		qscilexerhtml.cpp \
                qscilexerxml.cpp \
                qscilexerlua.cpp
}

SOURCES += \
	qscimacro.cpp \
	qsciprinter.cpp \
	qscistyle.cpp \
	qscistyledtext.cpp \
	MacPasteboardMime.cpp \
	InputMethod.cpp \
	SciClasses.cpp \
	ListBoxQt.cpp \
	PlatQt.cpp \
	ScintillaQt.cpp

contains(ALL_LEXERS, true) {
	SOURCES += \
	../lexers/LexA68k.cpp \
	../lexers/LexAbaqus.cpp \
	../lexers/LexAda.cpp \
	../lexers/LexAPDL.cpp \
	../lexers/LexAsm.cpp \
	../lexers/LexAsn1.cpp \
	../lexers/LexASY.cpp \
	../lexers/LexAU3.cpp \
	../lexers/LexAVE.cpp \
	../lexers/LexAVS.cpp \
	../lexers/LexBaan.cpp \
	../lexers/LexBash.cpp \
	../lexers/LexBasic.cpp \
	../lexers/LexBibTex.cpp \
	../lexers/LexBullant.cpp \
	../lexers/LexCaml.cpp \
	../lexers/LexCLW.cpp \
	../lexers/LexCmake.cpp \
	../lexers/LexCOBOL.cpp \
	../lexers/LexCoffeeScript.cpp \
	../lexers/LexConf.cpp \
	../lexers/LexCPP.cpp \
	../lexers/LexCrontab.cpp \
	../lexers/LexCsound.cpp \
	../lexers/LexCSS.cpp \
	../lexers/LexD.cpp \
	../lexers/LexDMAP.cpp \
	../lexers/LexDMIS.cpp \
	../lexers/LexECL.cpp \
	../lexers/LexEiffel.cpp \
	../lexers/LexErlang.cpp \
	../lexers/LexEScript.cpp \
	../lexers/LexFlagship.cpp \
	../lexers/LexForth.cpp \
	../lexers/LexFortran.cpp \
	../lexers/LexGAP.cpp \
	../lexers/LexGui4Cli.cpp \
	../lexers/LexHaskell.cpp \
	../lexers/LexHex.cpp \
	../lexers/LexHTML.cpp \
	../lexers/LexInno.cpp \
	../lexers/LexKix.cpp \
	../lexers/LexKVIrc.cpp \
	../lexers/LexLaTex.cpp \
	../lexers/LexLisp.cpp \
        ../lexers/LexLout.cpp \
        ../lexers/LexLua.cpp \
	../lexers/LexMagik.cpp \
	../lexers/LexMarkdown.cpp \
	../lexers/LexMatlab.cpp \
	../lexers/LexMetapost.cpp \
	../lexers/LexMMIXAL.cpp \
	../lexers/LexModula.cpp \
	../lexers/LexMPT.cpp \
	../lexers/LexMSSQL.cpp \
	../lexers/LexMySQL.cpp \
	../lexers/LexNimrod.cpp \
	../lexers/LexNsis.cpp \
	../lexers/LexOpal.cpp \
	../lexers/LexOScript.cpp \
	../lexers/LexOthers.cpp \
	../lexers/LexPascal.cpp \
	../lexers/LexPB.cpp \
	../lexers/LexPerl.cpp \
	../lexers/LexPLM.cpp \
	../lexers/LexPO.cpp \
	../lexers/LexPOV.cpp \
	../lexers/LexPowerPro.cpp \
	../lexers/LexPowerShell.cpp \
	../lexers/LexProgress.cpp \
	../lexers/LexPS.cpp \
	../lexers/LexPython.cpp \
	../lexers/LexR.cpp \
	../lexers/LexRebol.cpp \
	../lexers/LexRegistry.cpp \
	../lexers/LexRuby.cpp \
	../lexers/LexRust.cpp \
	../lexers/LexScriptol.cpp \
	../lexers/LexSmalltalk.cpp \
	../lexers/LexSML.cpp \
	../lexers/LexSorcus.cpp \
	../lexers/LexSpecman.cpp \
	../lexers/LexSpice.cpp \
	../lexers/LexSQL.cpp \
	../lexers/LexSTTXT.cpp \
	../lexers/LexTACL.cpp \
	../lexers/LexTADS3.cpp \
	../lexers/LexTAL.cpp \
	../lexers/LexTCL.cpp \
	../lexers/LexTCMD.cpp \
	../lexers/LexTeX.cpp \
	../lexers/LexTxt2tags.cpp \
	../lexers/LexVB.cpp \
	../lexers/LexVerilog.cpp \
	../lexers/LexVHDL.cpp \
	../lexers/LexVisualProlog.cpp \
	../lexers/LexYAML.cpp
}
else {
	SOURCES += \
	../lexers/LexPython.cpp \
	../lexers/LexCPP.cpp \
	../lexers/LexHTML.cpp \
	../lexers/LexLua.cpp
}

SOURCES += \
	../lexlib/Accessor.cpp \
	../lexlib/CharacterCategory.cpp \
	../lexlib/CharacterSet.cpp \
	../lexlib/LexerBase.cpp \
	../lexlib/LexerModule.cpp \
	../lexlib/LexerNoExceptions.cpp \
	../lexlib/LexerSimple.cpp \
	../lexlib/PropSetSimple.cpp \
	../lexlib/StyleContext.cpp \
	../lexlib/WordList.cpp \
	../src/AutoComplete.cpp \
	../src/CallTip.cpp \
	../src/CaseConvert.cpp \
	../src/CaseFolder.cpp \
	../src/Catalogue.cpp \
	../src/CellBuffer.cpp \
	../src/CharClassify.cpp \
	../src/ContractionState.cpp \
	../src/Decoration.cpp \
	../src/Document.cpp \
	../src/EditModel.cpp \
	../src/Editor.cpp \
	../src/EditView.cpp \
	../src/ExternalLexer.cpp \
	../src/Indicator.cpp \
    ../src/KeyMap.cpp \
	../src/LineMarker.cpp \
	../src/MarginView.cpp \
	../src/PerLine.cpp \
	../src/PositionCache.cpp \
    ../src/RESearch.cpp \
	../src/RunStyles.cpp \
    ../src/ScintillaBase.cpp \
    ../src/Selection.cpp \
	../src/Style.cpp \
	../src/UniConversion.cpp \
	../src/ViewStyle.cpp \
	../src/XPM.cpp

TRANSLATIONS = \
	qscintilla_cs.ts \
	qscintilla_de.ts \
	qscintilla_es.ts \
	qscintilla_fr.ts \
	qscintilla_pt_br.ts

# Uses FORCERELEASE variable because CONFIG and SUBDIR force three executions
# if qmake and the last one does not preserves CONFIG from command line.
contains(FORCERELEASE, true) {
  CONFIG += qt warn_on release
  CONFIG -= debug
  DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_DEBUG_WARNING
  message ("plugins-common.pri RELEASE build!")
}
else {
  CONFIG += qt warn_on debug console
  CONFIG -= release
  message ("plugins-common.pri DEBUG build!")
}

isEmpty(DESTDIR) {
  release: DESTDIR = $${PWD}/../../../../../../bin/release/plugins/
  debug:   DESTDIR = $${PWD}/../../../../../../bin/debug/plugins/
}

#OBJECTS_DIR = $$DESTDIR/.obj
#MOC_DIR = $$DESTDIR/.moc
#RCC_DIR = $$DESTDIR/.qrc
#UI_DIR = $$DESTDIR/.ui
