IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_azoth)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTDBUS TRUE)
SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTWEBKIT TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	azoth.cpp
	core.cpp
	mainwidget.cpp
	chattabsmanager.cpp
	chattab.cpp
	sortfilterproxymodel.cpp
	accountslistdialog.cpp
	setstatusdialog.cpp
	pluginmanager.cpp
	proxyobject.cpp
	textedit.cpp
	xmlsettingsmanager.cpp
	contactlistdelegate.cpp
	addcontactdialog.cpp
	joinconferencedialog.cpp
	groupeditordialog.cpp
	transferjobmanager.cpp
	addaccountwizardfirstpage.cpp
	bookmarksmanagerdialog.cpp
	accounthandlerchooserdialog.cpp
	util.cpp
	)
SET (HEADERS
	azoth.h
	core.h
	mainwidget.h
	chattabsmanager.h
	chattab.h
	sortfilterproxymodel.h
	accountslistdialog.h
	setstatusdialog.h
	pluginmanager.h
	proxyobject.h
	textedit.h
	xmlsettingsmanager.h
	contactlistdelegate.h
	addcontactdialog.h
	joinconferencedialog.h
	groupeditordialog.h
	transferjobmanager.h
	addaccountwizardfirstpage.h
	bookmarksmanagerdialog.h
	accounthandlerchooserdialog.h
	util.h
	sourcetrackingmodel.h
	interfaces/iprotocolplugin.h
	interfaces/iaccount.h
	interfaces/iprotocol.h
	interfaces/iclentry.h
	)
SET (FORMS
	mainwidget.ui
	chattab.ui
	accountslistdialog.ui
	setstatusdialog.ui
	addcontactdialog.ui
	joinconferencedialog.ui
	groupeditordialog.ui
	addaccountwizardfirstpage.ui
	bookmarksmanagerdialog.ui
	accounthandlerchooserdialog.ui
	)
SET (RESOURCES azothresources.qrc)
SET (TRANSLATIONS
	leechcraft_azoth_ru_RU.ts
	leechcraft_azoth_uk_UA.ts
	leechcraft_azoth_en.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_azoth_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_azoth_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_azoth_en.qm
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES azothsettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY share/azoth DESTINATION share/leechcraft)

SET (AZOTH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

OPTION (ENABLE_AZOTH_ACETAMIDE "Build Acetamide, IRC support for Azoth" ON)
OPTION (ENABLE_AZOTH_CHATHISTORY "Build ChatHistory which keeps, well, chat history" ON)
OPTION (ENABLE_AZOTH_NATIVEEMOTICONS "Build support for Azoth's own emoticons packs" ON)
OPTION (ENABLE_AZOTH_STANDARDSTYLES "Build support for standard Azoth styles" ON)
OPTION (ENABLE_AZOTH_P100Q "Build plugin for the psto.net microblogging service" ON)
OPTION (ENABLE_AZOTH_JUICK "Build plugin for the juick.com microblogging service" ON)
OPTION (ENABLE_AZOTH_XOOX "Build Xoox, the QXmpp-aided XMPP support for Azoth" ON)

IF (ENABLE_AZOTH_ACETAMIDE)
	ADD_SUBDIRECTORY (plugins/acetamide)
ENDIF (ENABLE_AZOTH_ACETAMIDE)
IF (ENABLE_AZOTH_CHATHISTORY)
	ADD_SUBDIRECTORY (plugins/chathistory)
ENDIF (ENABLE_AZOTH_CHATHISTORY)
IF (ENABLE_AZOTH_NATIVEEMOTICONS)
	ADD_SUBDIRECTORY (plugins/nativeemoticons)
ENDIF (ENABLE_AZOTH_NATIVEEMOTICONS)
IF (ENABLE_AZOTH_STANDARDSTYLES)
	ADD_SUBDIRECTORY (plugins/standardstyles)
ENDIF (ENABLE_AZOTH_STANDARDSTYLES)
IF (ENABLE_AZOTH_P100Q)
	ADD_SUBDIRECTORY (plugins/p100q)
ENDIF (ENABLE_AZOTH_P100Q)
IF (ENABLE_AZOTH_JUICK)
	ADD_SUBDIRECTORY (plugins/juick)
ENDIF (ENABLE_AZOTH_JUICK)
IF (ENABLE_AZOTH_XOOX)
	ADD_SUBDIRECTORY (plugins/xoox)
ENDIF (ENABLE_AZOTH_XOOX)
