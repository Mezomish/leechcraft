/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDockWidget>
#include <QLabel>
#include <QList>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QCloseEvent>
#include <QShowEvent>
#include <QTabBar>
#include <QToolBar>
#include <QHash>
#include <QSessionManager>
#include <QShortcut>
#include <QKeySequence>
#include <QToolButton>
#include <QRegExp>
#include <QTreeView>
#include <QMetaType>
#include <QTimer>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/DownloadManager.h"
#include "dcpp/LogManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/TimerManager.h"
#include "dcpp/UploadManager.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Download.h"
#include "dcpp/Util.h"
#include "dcpp/version.h"

#include <interfaces/iinfo.h>
#include <interfaces/imultitabs.h>

#include "qtsingleapp/qtlockedfile.h"
#include "qtsingleapp/qtsingleapplication.h"

#include "ArenaWidget.h"
#include "ArenaWidgetContainer.h"
#include "HistoryInterface.h"
#include "LineEdit.h"
#include "WulforSettings.h"
#include "ShortcutManager.h"
#include "ToolBar.h"

#include "ui_UIAbout.h"

class FavoriteHubs;
class DownloadQueue;
class MainLayout;
class MultiLineToolBar;

extern const char * const EISKALTDCPP_VERSION;
extern const char * const EISKALTDCPP_WND_TITLE;
extern const char * const EISKALTDCPP_VERSION_SFX;

class QProgressBar;

class About:
        public QDialog,
        public Ui::UIAbout
{
Q_OBJECT

public:
    About(QWidget *parent = NULL): QDialog(parent){ setupUi(this); }

    void printHelp() const {
        QString msg =   tr("Using:\n"
                        "  eiskaltdcpp-qt <magnet link> <dchub://link> <adc(s)://link>\n"
                        "  eiskaltdcpp-qt <Key>\n"
                        "EiskaltDC++ is a cross-platform program that uses the Direct Connect and ADC protocol.\n"
                        "\n"
                        "Keys:\n"
                        "  -h, --help\t Show this message\n"
                        "  -v, --version\t Show version string"
                        );

        printf("%s\n", msg.toUtf8().constData());
    }

    void printVersion() const {
        printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
    }
};

class HashProgress;

class MainLayout:
        public QMainWindow,
        public dcpp::Singleton<MainLayout>,
        public IInfo,
        public IMultiTabs,
        private dcpp::LogManagerListener,
        private dcpp::TimerManagerListener,
        private dcpp::QueueManagerListener
{
    Q_OBJECT
    Q_INTERFACES (IInfo IMultiTabs)

friend class dcpp::Singleton<MainLayout>;

    public:

        typedef QList<QAction*> ActionList;
        typedef QList<ArenaWidget*> ArenaWidgetList;
        typedef QMap<ArenaWidget*, QWidget*> ArenaWidgetMap;

        MainLayout (QWidget *parent=NULL);
        virtual ~MainLayout();

        QToolBar *getActionBar() { return qobject_cast<QToolBar*>(fBar); }

        void beginExit();

        /** */
        void newHubFrame(QString, QString);

        /** */
        void browseOwnFiles();

        /** */
        void startSocket(bool changed);
        /** */
        void showPortsError(const std::string& port);
        /** */
        void autoconnect();
        /** */
        void retranslateUi();

        /** */
        void reloadSomeSettings();

        /** */
        void setUnload(bool b){ isUnload = b; }

        ArenaWidget *widgetForRole(ArenaWidget::Role) const;

        /** IInfo interface */
        virtual void Init (ICoreProxy_ptr proxy);
        virtual void SecondInit ();
        virtual QByteArray GetUniqueID () const;
        virtual QString GetName () const;
        virtual QString GetInfo () const;
        virtual void Release ();
        virtual QIcon GetIcon () const;
        QStringList Provides () const { return QStringList ("directconnect"); }

    public Q_SLOTS:
        /** Allow widget to be mapped on arena*/
        void addArenaWidget(ArenaWidget*);
        /** Disallow widget to be mapped on arena*/
        void remArenaWidget(ArenaWidget*);
        /** Show widget on arena */
        void mapWidgetOnArena(ArenaWidget*);
        /** Remove widget from arena*/
        void remWidgetFromArena(ArenaWidget*);

        /** */
        void addArenaWidgetOnToolbar(ArenaWidget*, bool keepFocus = false);
        /** */
        void remArenaWidgetFromToolbar(ArenaWidget*);

        QObject *getToolBar();

        /** */
        void addActionOnToolBar(QAction*);
        /** */
        void remActionFromToolBar(QAction*);

        /** */
        void toggleMainMenu(bool);

        void slotChatClear();

        /** */
        void redrawToolPanel();

        /** */
        void setStatusMessage(QString);

        /** */
        void show();

        /** */
        void parseCmdLine();
        /** */
        void parseInstanceLine(QString);

        /** IMultiTabs interface*/
        virtual void newTabRequested(){}

Q_SIGNALS:
        /** IMultiTabs interface*/
        void addNewTab(const QString &name, QWidget *tabContents);
        void removeTab (QWidget *tabContents);
        void changeTabName (QWidget *tabContents, const QString& name);
        void changeTabIcon (QWidget *tabContents, const QIcon& icon);
        void statusBarChanged (QWidget *tabContents, const QString& text);
        void raiseTab (QWidget *tabContents);

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void showEvent(QShowEvent *);
        virtual void hideEvent(QHideEvent *);
        virtual bool eventFilter(QObject *, QEvent *);

    private Q_SLOTS:
        void slotOpenMagnet();
        void slotFileOpenLogFile();
        void slotFileOpenDownloadDirectory();
        void slotFileBrowseFilelist();
        void slotFileHasher();
        void slotFileBrowseOwnFilelist();
        void slotFileHashProgress();
        void slotFileRefreshShareHashProgress();
        void slotHubsReconnect();
        void slotHubsFavoriteHubs();
        void slotHubsPublicHubs();
        void slotHubsFavoriteUsers();
        void slotToolsDownloadQueue();
        void slotToolsQueuedUsers();
        void slotToolsHubManager();
        void slotToolsFinishedDownloads();
        void slotToolsFinishedUploads();
        void slotToolsSpy();
        void slotToolsAntiSpam();
        void slotToolsIPFilter();
        void slotToolsSwitchAway();
        void slotToolsAutoAway();
        void slotToolsSearch();
        void slotToolsADLS();
        void slotToolsCopyWindowTitle();
        void slotToolsSettings();
        void slotToolsJS();
        void slotToolsJSConsole();
        void slotToolsTransfer(bool);
        void slotToolsSwitchSpeedLimit();
        void slotPanelMenuActionClicked();
        void slotWidgetsToggle();
        void slotQC();
        void slotHideMainMenu();
        void slotShowMainMenu();
        void slotHideWindow();
        void slotHideProgressSpace();
        void slotHideLastStatus();
        void slotHideUsersStatistics();
        void slotSidebarContextMenu();
        void slotSidebarHook(const QModelIndex&);
        void slotSideBarDblClicked(const QModelIndex&);
        void slotSideBarDockMenu();
        void slotSelectSidebarIndex(const QModelIndex&);
        void slotExit();
        void slotToolbarCustomization();
        void slotToolbarCustomizerDone(const QList<QAction*> &enabled);

        void slotCloseCurrentWidget();

        void slotUnixSignal(int);

        void nextMsg();
        void prevMsg();

        void slotFind();
        void slotChatDisable();

        void slotAboutOpenUrl();
        void slotAboutClient();
        void slotAboutQt();

        void showShareBrowser(dcpp::UserPtr, const QString &, const QString&);
        void updateStatus(const QMap<QString,QString> &);

    Q_SIGNALS:
        void coreLogMessage(const QString&);
        void coreOpenShare(dcpp::UserPtr, const QString &, const QString&);
        void coreUpdateStats(const QMap<QString, QString> &);
        void notifyMessage(int, const QString&, const QString&);

    private:
         /** LogManagerListener */
        virtual void on(dcpp::LogManagerListener::Message, time_t t, const std::string&) throw();
        /** TimerManagerListener */
        virtual void on(dcpp::TimerManagerListener::Second, uint64_t) throw();
        /** QueueManagerListener */
        virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem*, const std::string&, int64_t) throw();

        // Interface setup functions
        void init();
        void loadSettings();
        void saveSettings();

        void initActions();
        void initMenuBar();
        void initStatusBar();
        void initSearchBar();
        void initToolbar();
        void initSideBar();

        void toggleSingletonWidget(ArenaWidget *a);

        void updateHashProgressStatus();

        HashProgress *progress_dialog(); // Lazy initialization for _progress_dialog;
        bool isUnload;
        bool exitBegin;

        // position and geometry
        bool showMax;
        int w;
        int h;
        int xPos;
        int yPos;

        //Statistics
        qulonglong totalUp;
        qulonglong totalDown;

        // Widgets
        QDockWidget *arena;
        QDockWidget *transfer_dock;
        QDockWidget *sideDock;

        QTreeView *sideTree;

        ArenaWidgetContainer *wcontainer;

        MultiLineToolBar *mBar; // multi-line ToolBar
        ToolBar *tBar; // default ToolBar
        ToolBar *fBar; //for actions
        ToolBar *sBar; //for fast search

        LineEdit   *searchLineEdit;
        QStringList core_msg_history;
        QLabel *statusLabel;
        QLabel *statusSPLabel;
        QLabel *statusDLabel;
        QLabel *statusTRLabel;
        QLabel *msgLabel;
        QProgressBar *progressSpace;
        QProgressBar *progressHashing;
        HashProgress *_progress_dialog; // Hashing progress dialog

        QMenu   *menuFile;
        QAction *fileOpenMagnet;
        QAction *fileFileListBrowser;
        QAction *fileFileHasher;
        QAction *fileFileListBrowserLocal;
        QAction *fileRefreshShareHashProgress;
        QAction *fileOpenLogFile;
        QAction *fileOpenDownloadDirectory;
        QAction *fileHideWindow;
        QAction *fileQuit;

        QMenu   *menuHubs;
        QAction *hubsHubReconnect;
        QAction *hubsQuickConnect;
        QAction *hubsFavoriteHubs;
        QAction *hubsPublicHubs;
        QAction *hubsFavoriteUsers;

        QMenu   *menuTools;
        QAction *toolsSearch;
        QAction *toolsADLS;
        QAction *toolsTransfers;
        QAction *toolsDownloadQueue;
        QAction *toolsQueuedUsers;
        QAction *toolsFinishedDownloads;
        QAction *toolsFinishedUploads;
        QAction *toolsSpy;
        QAction *toolsAntiSpam;
        QAction *toolsIPFilter;
        QAction *menuAwayAction;
        QAction *toolsHubManager;
        // submenu
        QMenu   *menuAway;
        QActionGroup *awayGroup;
        QAction *toolsAwayOn;
        QAction *toolsAwayOff;
        QAction *toolsAutoAway;
        // end
        QAction *toolsHideProgressSpace;
        QAction *toolsHideLastStatus;
        QAction *toolsHideUsersStatisctics;
        QAction *toolsCopyWindowTitle;
        QAction *toolsOptions;
#ifdef USE_JS
        QAction *toolsJS;
        QAction *toolsJSConsole;
#endif
        QAction *toolsSwitchSpeedLimit;

        QMenu   *menuPanels;
        // submenu
        QMenu   *sh_menu;
        // end
        QAction *panelsWidgets;
        QAction *panelsTools;
        QAction *panelsSearch;

        // Standalone shortcuts
        QAction *prevTabShortCut;
        QAction *nextTabShortCut;
        QAction *prevMsgShortCut;
        QAction *nextMsgShortCut;
        QAction *closeWidgetShortCut;
        QAction *toggleMainMenuShortCut;

        QAction *chatDisable;
        QAction *findInWidget;
        QAction *chatClear;

        QMenu *menuWidgets;
        QList<QAction*> menuWidgetsActions;
        QHash<QAction*, ArenaWidget*> menuWidgetsHash;

        QMenu   *menuAbout;
        QAction *aboutHomepage;
        QAction *aboutSource;
        QAction *aboutIssues;
        QAction *aboutWiki;
        QAction *aboutChangelog;
        QAction *aboutClient;
        QAction *aboutQt;

        ActionList toolBarActions;
        ActionList fileMenuActions;
        ActionList hubsMenuActions;
        ActionList toolsMenuActions;
        ArenaWidgetList arenaWidgets;
        ArenaWidgetMap arenaMap;

        QMainWindow *_lc_MW;
};

Q_DECLARE_METATYPE(MainLayout*)


#endif //MAINWINDOW_H_
