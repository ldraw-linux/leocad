#include "lc_global.h"
#include <typeinfo>
#include "lc_qmainwindow.h"
#include "lc_qutils.h"
#include "lc_glwidget.h"
#include "lc_library.h"
#include "lc_application.h"
#include "pieceinf.h"
#include "project.h"
#include "preview.h"
#include "camera.h"
#include "view.h"
#include "lc_qpartstree.h"
#include "lc_colorlistwidget.h"
#include "keyboard.h"
#include "system.h"
#include "mainwnd.h"
#include "lc_profile.h"

lcQMainWindow::lcQMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	memset(actions, 0, sizeof(actions));

	setWindowFilePath(QString());

	resize(800, 600);
//	centralWidget = new lcViewWidget(this, NULL);
//	centralWidget->mWindow->OnInitialUpdate();
//	setCentralWidget(centralWidget);

	createActions();
	createToolBars();
	createMenus();
	createStatusBar();

	QFrame *previewFrame = new QFrame;
	previewFrame->setFrameShape(QFrame::StyledPanel);
	previewFrame->setFrameShadow(QFrame::Sunken);
	setCentralWidget(previewFrame);

	QGridLayout *previewLayout = new QGridLayout(previewFrame);
	previewLayout->setContentsMargins(0, 0, 0, 0);

	QWidget *viewWidget = new lcGLWidget(previewFrame, piecePreview, new View(lcGetActiveProject()), true);
	previewLayout->addWidget(viewWidget, 0, 0, 1, 1);

	connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));
	clipboardChanged();

	lcPiecesLibrary* Library = lcGetPiecesLibrary();
	PieceInfo* Info = Library->FindPiece("3005", false);

	if (!Info)
		Info = Library->mPieces[0];

	if (Info)
	{
		lcGetActiveProject()->SetCurrentPiece(Info);
		PiecePreview* Preview = (PiecePreview*)piecePreview->mWindow;
		gMainWindow->mPreviewWidget = Preview;
		Preview->SetCurrentPiece(Info);
	}
}

lcQMainWindow::~lcQMainWindow()
{
}

void lcQMainWindow::createActions()
{
	for (int Command = 0; Command < LC_NUM_COMMANDS; Command++)
	{
		QAction *action = new QAction(tr(gActions[Command].MenuName), this);
		action->setStatusTip(tr(gActions[Command].StatusText));
		connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
		addAction(action);
		actions[Command] = action;
	}

	actions[LC_FILE_NEW]->setToolTip(tr("New Project"));
	actions[LC_FILE_OPEN]->setToolTip(tr("Open Project"));
	actions[LC_FILE_SAVE]->setToolTip(tr("Save Project"));

	actions[LC_FILE_NEW]->setIcon(QIcon(":/resources/file_new.png"));
	actions[LC_FILE_OPEN]->setIcon(QIcon(":/resources/file_open.png"));
	actions[LC_FILE_SAVE]->setIcon(QIcon(":/resources/file_save.png"));
	actions[LC_FILE_PRINT]->setIcon(QIcon(":/resources/file_print.png"));
	actions[LC_FILE_PRINT_PREVIEW]->setIcon(QIcon(":/resources/file_print_preview.png"));
	actions[LC_EDIT_UNDO]->setIcon(QIcon(":/resources/edit_undo.png"));
	actions[LC_EDIT_REDO]->setIcon(QIcon(":/resources/edit_redo.png"));
	actions[LC_EDIT_CUT]->setIcon(QIcon(":/resources/edit_cut.png"));
	actions[LC_EDIT_COPY]->setIcon(QIcon(":/resources/edit_copy.png"));
	actions[LC_EDIT_PASTE]->setIcon(QIcon(":/resources/edit_paste.png"));
	actions[LC_EDIT_LOCK_TOGGLE]->setIcon(QIcon(":/resources/edit_lock.png"));
	actions[LC_EDIT_SNAP_TOGGLE]->setIcon(QIcon(":/resources/edit_snap_move.png"));
	actions[LC_EDIT_SNAP_ANGLE]->setIcon(QIcon(":/resources/edit_snap_angle.png"));
	actions[LC_EDIT_TRANSFORM]->setIcon(QIcon(":/resources/edit_transform.png"));
	actions[LC_EDIT_ACTION_INSERT]->setIcon(QIcon(":/resources/action_insert.png"));
	actions[LC_EDIT_ACTION_LIGHT]->setIcon(QIcon(":/resources/action_light.png"));
	actions[LC_EDIT_ACTION_SPOTLIGHT]->setIcon(QIcon(":/resources/action_spotlight.png"));
	actions[LC_EDIT_ACTION_CAMERA]->setIcon(QIcon(":/resources/action_camera.png"));
	actions[LC_EDIT_ACTION_SELECT]->setIcon(QIcon(":/resources/action_select.png"));
	actions[LC_EDIT_ACTION_MOVE]->setIcon(QIcon(":/resources/action_move.png"));
	actions[LC_EDIT_ACTION_ROTATE]->setIcon(QIcon(":/resources/action_rotate.png"));
	actions[LC_EDIT_ACTION_DELETE]->setIcon(QIcon(":/resources/action_delete.png"));
	actions[LC_EDIT_ACTION_PAINT]->setIcon(QIcon(":/resources/action_paint.png"));
	actions[LC_EDIT_ACTION_ZOOM]->setIcon(QIcon(":/resources/action_zoom.png"));
	actions[LC_EDIT_ACTION_PAN]->setIcon(QIcon(":/resources/action_pan.png"));
	actions[LC_EDIT_ACTION_ROTATE_VIEW]->setIcon(QIcon(":/resources/action_rotate_view.png"));
	actions[LC_EDIT_ACTION_ROLL]->setIcon(QIcon(":/resources/action_roll.png"));
	actions[LC_EDIT_ACTION_ZOOM_REGION]->setIcon(QIcon(":/resources/action_zoom_region.png"));
	actions[LC_VIEW_ZOOM_IN]->setIcon(QIcon(":/resources/view_zoomin.png"));
	actions[LC_VIEW_ZOOM_OUT]->setIcon(QIcon(":/resources/view_zoomout.png"));
	actions[LC_VIEW_ZOOM_EXTENTS]->setIcon(QIcon(":/resources/view_zoomextents.png"));
	actions[LC_VIEW_TIME_FIRST]->setIcon(QIcon(":/resources/time_first.png"));
	actions[LC_VIEW_TIME_PREVIOUS]->setIcon(QIcon(":/resources/time_previous.png"));
	actions[LC_VIEW_TIME_NEXT]->setIcon(QIcon(":/resources/time_next.png"));
	actions[LC_VIEW_TIME_LAST]->setIcon(QIcon(":/resources/time_last.png"));
	actions[LC_HELP_HOMEPAGE]->setIcon(QIcon(":/resources/help_homepage.png"));
	actions[LC_HELP_EMAIL]->setIcon(QIcon(":/resources/help_email.png"));

	actions[LC_EDIT_LOCK_X]->setCheckable(true);
	actions[LC_EDIT_LOCK_Y]->setCheckable(true);
	actions[LC_EDIT_LOCK_Z]->setCheckable(true);
	actions[LC_EDIT_SNAP_X]->setCheckable(true);
	actions[LC_EDIT_SNAP_Y]->setCheckable(true);
	actions[LC_EDIT_SNAP_Z]->setCheckable(true);
	actions[LC_VIEW_CAMERA_NONE]->setCheckable(true);

	QActionGroup *actionSnapXYGroup = new QActionGroup(this);
	for (int actionIdx = LC_EDIT_SNAP_MOVE_XY0; actionIdx <= LC_EDIT_SNAP_MOVE_XY9; actionIdx++)
	{
		actions[actionIdx]->setCheckable(true);
		actionSnapXYGroup->addAction(actions[actionIdx]);
	}

	QActionGroup *actionSnapZGroup = new QActionGroup(this);
	for (int actionIdx = LC_EDIT_SNAP_MOVE_Z0; actionIdx <= LC_EDIT_SNAP_MOVE_Z9; actionIdx++)
	{
		actions[actionIdx]->setCheckable(true);
		actionSnapZGroup->addAction(actions[actionIdx]);
	}

	QActionGroup *actionSnapAngleGroup = new QActionGroup(this);
	for (int actionIdx = LC_EDIT_SNAP_ANGLE0; actionIdx <= LC_EDIT_SNAP_ANGLE9; actionIdx++)
	{
		actions[actionIdx]->setCheckable(true);
		actionSnapAngleGroup->addAction(actions[actionIdx]);
	}

	QActionGroup *actionTransformTypeGroup = new QActionGroup(this);
	for (int actionIdx = LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION; actionIdx <= LC_EDIT_TRANSFORM_RELATIVE_ROTATION; actionIdx++)
	{
		actions[actionIdx]->setCheckable(true);
		actionTransformTypeGroup->addAction(actions[actionIdx]);
	}

	QActionGroup *actionToolGroup = new QActionGroup(this);
	for (int actionIdx = LC_EDIT_ACTION_FIRST; actionIdx <= LC_EDIT_ACTION_LAST; actionIdx++)
	{
		actions[actionIdx]->setCheckable(true);
		actionToolGroup->addAction(actions[actionIdx]);
	}

	QActionGroup *actionCameraGroup = new QActionGroup(this);
	actionCameraGroup->addAction(actions[LC_VIEW_CAMERA_NONE]);
	for (int actionIdx = LC_VIEW_CAMERA_FIRST; actionIdx <= LC_VIEW_CAMERA_LAST; actionIdx++)
	{
		actions[actionIdx]->setCheckable(true);
		actionCameraGroup->addAction(actions[actionIdx]);
	}

	updateShortcuts();
}

void lcQMainWindow::createMenus()
{
	QMenu* lockMenu = new QMenu(tr("Lock Menu"));
	lockMenu->addAction(actions[LC_EDIT_LOCK_X]);
	lockMenu->addAction(actions[LC_EDIT_LOCK_Y]);
	lockMenu->addAction(actions[LC_EDIT_LOCK_Z]);
	lockMenu->addAction(actions[LC_EDIT_LOCK_NONE]);
	actions[LC_EDIT_LOCK_TOGGLE]->setMenu(lockMenu);

	QMenu* snapXYMenu = new QMenu(tr("Snap XY"));
	for (int actionIdx = LC_EDIT_SNAP_MOVE_XY0; actionIdx <= LC_EDIT_SNAP_MOVE_XY9; actionIdx++)
		snapXYMenu->addAction(actions[actionIdx]);

	QMenu* snapZMenu = new QMenu(tr("Snap Z"));
	for (int actionIdx = LC_EDIT_SNAP_MOVE_Z0; actionIdx <= LC_EDIT_SNAP_MOVE_Z9; actionIdx++)
		snapZMenu->addAction(actions[actionIdx]);

	QMenu* snapMenu = new QMenu(tr("Snap Menu"));
	snapMenu->addMenu(snapXYMenu);
	snapMenu->addMenu(snapZMenu);
	snapMenu->addSeparator();
	snapMenu->addAction(actions[LC_EDIT_SNAP_X]);
	snapMenu->addAction(actions[LC_EDIT_SNAP_Y]);
	snapMenu->addAction(actions[LC_EDIT_SNAP_Z]);
	snapMenu->addAction(actions[LC_EDIT_SNAP_NONE]);
	snapMenu->addAction(actions[LC_EDIT_SNAP_ALL]);
	actions[LC_EDIT_SNAP_TOGGLE]->setMenu(snapMenu);

	QMenu* snapAngleMenu = new QMenu(tr("Snap Angle Menu"));
	for (int actionIdx = LC_EDIT_SNAP_ANGLE0; actionIdx <= LC_EDIT_SNAP_ANGLE9; actionIdx++)
		snapAngleMenu->addAction(actions[actionIdx]);
	actions[LC_EDIT_SNAP_ANGLE]->setMenu(snapAngleMenu);

	QMenu* transformMenu = new QMenu(tr("Transform"));
	transformMenu->addAction(actions[LC_EDIT_TRANSFORM_RELATIVE_TRANSLATION]);
	transformMenu->addAction(actions[LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION]);
	transformMenu->addAction(actions[LC_EDIT_TRANSFORM_RELATIVE_ROTATION]);
	transformMenu->addAction(actions[LC_EDIT_TRANSFORM_ABSOLUTE_ROTATION]);
	actions[LC_EDIT_TRANSFORM]->setMenu(transformMenu);

	menuCamera = new QMenu(tr("Cameras"));
	menuCamera->addAction(actions[LC_VIEW_CAMERA_NONE]);

	for (int actionIdx = LC_VIEW_CAMERA_FIRST; actionIdx <= LC_VIEW_CAMERA_LAST; actionIdx++)
		menuCamera->addAction(actions[actionIdx]);

	menuCamera->addSeparator();
	menuCamera->addAction(actions[LC_VIEW_CAMERA_RESET]);

	menuFile = menuBar()->addMenu(tr("&File"));
	menuFile->addAction(actions[LC_FILE_NEW]);
	menuFile->addAction(actions[LC_FILE_OPEN]);
	menuFile->addAction(actions[LC_FILE_MERGE]);
	menuFile->addSeparator();
	menuFile->addAction(actions[LC_FILE_SAVE]);
	menuFile->addAction(actions[LC_FILE_SAVEAS]);
	menuFile->addAction(actions[LC_FILE_SAVE_IMAGE]);
	QMenu* exportMenu = menuFile->addMenu(tr("&Export"));
	exportMenu->addAction(actions[LC_FILE_EXPORT_3DS]);
	exportMenu->addAction(actions[LC_FILE_EXPORT_BRICKLINK]);
	exportMenu->addAction(actions[LC_FILE_EXPORT_CSV]);
	exportMenu->addAction(actions[LC_FILE_EXPORT_HTML]);
	exportMenu->addAction(actions[LC_FILE_EXPORT_POVRAY]);
	exportMenu->addAction(actions[LC_FILE_EXPORT_WAVEFRONT]);
	menuFile->addSeparator();
	menuFile->addAction(actions[LC_FILE_PROPERTIES]);
	menuFile->addAction(actions[LC_FILE_TERRAIN_EDITOR]);
	menuFile->addSeparator();
	menuFile->addAction(actions[LC_FILE_PRINT]);
	menuFile->addAction(actions[LC_FILE_PRINT_PREVIEW]);
	menuFile->addAction(actions[LC_FILE_PRINT_BOM]);
	menuFile->addSeparator();
	menuFile->addAction(actions[LC_FILE_RECENT1]);
	menuFile->addAction(actions[LC_FILE_RECENT2]);
	menuFile->addAction(actions[LC_FILE_RECENT3]);
	menuFile->addAction(actions[LC_FILE_RECENT4]);
	actionFileRecentSeparator = menuFile->addSeparator();
	menuFile->addAction(actions[LC_FILE_EXIT]);

	menuEdit = menuBar()->addMenu(tr("&Edit"));
	menuEdit->addAction(actions[LC_EDIT_UNDO]);
	menuEdit->addAction(actions[LC_EDIT_REDO]);
	menuEdit->addSeparator();
	menuEdit->addAction(actions[LC_EDIT_CUT]);
	menuEdit->addAction(actions[LC_EDIT_COPY]);
	menuEdit->addAction(actions[LC_EDIT_PASTE]);
	menuEdit->addSeparator();
	menuEdit->addAction(actions[LC_EDIT_SELECT_ALL]);
	menuEdit->addAction(actions[LC_EDIT_SELECT_NONE]);
	menuEdit->addAction(actions[LC_EDIT_SELECT_INVERT]);
	menuEdit->addAction(actions[LC_EDIT_SELECT_BY_NAME]);

	menuView = menuBar()->addMenu(tr("&View"));
	menuView->addAction(actions[LC_VIEW_PREFERENCES]);
	menuView->addSeparator();
	menuView->addAction(actions[LC_VIEW_ZOOM_EXTENTS]);
	QMenu* menuViewpoints = menuView->addMenu(tr("Viewpoints"));
	menuViewpoints->addAction(actions[LC_VIEW_VIEWPOINT_FRONT]);
	menuViewpoints->addAction(actions[LC_VIEW_VIEWPOINT_BACK]);
	menuViewpoints->addAction(actions[LC_VIEW_VIEWPOINT_LEFT]);
	menuViewpoints->addAction(actions[LC_VIEW_VIEWPOINT_RIGHT]);
	menuViewpoints->addAction(actions[LC_VIEW_VIEWPOINT_TOP]);
	menuViewpoints->addAction(actions[LC_VIEW_VIEWPOINT_BOTTOM]);
	menuViewpoints->addAction(actions[LC_VIEW_VIEWPOINT_HOME]);
	menuView->addMenu(menuCamera);
	QMenu* menuStep = menuView->addMenu(tr("Step"));
	menuStep->addAction(actions[LC_VIEW_TIME_FIRST]);
	menuStep->addAction(actions[LC_VIEW_TIME_PREVIOUS]);
	menuStep->addAction(actions[LC_VIEW_TIME_NEXT]);
	menuStep->addAction(actions[LC_VIEW_TIME_LAST]);
	//LC_VIEW_TIME_STOP
	//LC_VIEW_TIME_PLAY
	menuStep->addSeparator();
	menuStep->addAction(actions[LC_VIEW_TIME_INSERT]);
	menuStep->addAction(actions[LC_VIEW_TIME_DELETE]);
	menuView->addSeparator();
	menuView->addAction(actions[LC_VIEW_SPLIT_HORIZONTAL]);
	menuView->addAction(actions[LC_VIEW_SPLIT_VERTICAL]);
	menuView->addAction(actions[LC_VIEW_REMOVE_VIEW]);
	menuView->addAction(actions[LC_VIEW_RESET_VIEWS]);
	menuView->addSeparator();
	QMenu *menuToolBars = menuView->addMenu(tr("Toolbars"));
	menuToolBars->addAction(partsToolBar->toggleViewAction());
	menuToolBars->addAction(propertiesToolBar->toggleViewAction());
	menuToolBars->addSeparator();
	menuToolBars->addAction(standardToolBar->toggleViewAction());
	menuToolBars->addAction(toolsToolBar->toggleViewAction());
	menuToolBars->addAction(timeToolBar->toggleViewAction());
	menuView->addAction(actions[LC_VIEW_FULLSCREEN]);

	menuPiece = menuBar()->addMenu(tr("&Piece"));
	menuPiece->addAction(actions[LC_PIECE_INSERT]);
	menuPiece->addAction(actions[LC_PIECE_DELETE]);
	menuPiece->addAction(actions[LC_PIECE_ARRAY]);
	menuPiece->addAction(actions[LC_PIECE_MINIFIG_WIZARD]);
	//	LC_PIECE_COPY_KEYS
	menuPiece->addSeparator();
	menuPiece->addAction(actions[LC_PIECE_GROUP]);
	menuPiece->addAction(actions[LC_PIECE_UNGROUP]);
	menuPiece->addAction(actions[LC_PIECE_GROUP_REMOVE]);
	menuPiece->addAction(actions[LC_PIECE_GROUP_ADD]);
	menuPiece->addAction(actions[LC_PIECE_GROUP_EDIT]);
//	LC_PIECE_SHOW_EARLIER,
//	LC_PIECE_SHOW_LATER,
	menuPiece->addSeparator();
	menuPiece->addAction(actions[LC_PIECE_HIDE_SELECTED]);
	menuPiece->addAction(actions[LC_PIECE_HIDE_UNSELECTED]);
	menuPiece->addAction(actions[LC_PIECE_UNHIDE_ALL]);

	menuHelp = menuBar()->addMenu(tr("&Help"));
	menuHelp->addAction(actions[LC_HELP_HOMEPAGE]);
	menuHelp->addAction(actions[LC_HELP_EMAIL]);
	menuHelp->addAction(actions[LC_HELP_UPDATES]);
	menuHelp->addSeparator();
	menuHelp->addAction(actions[LC_HELP_ABOUT]);
}

void lcQMainWindow::createToolBars()
{
	standardToolBar = addToolBar(tr("Standard"));
	standardToolBar->setObjectName("StandardToolbar");
	standardToolBar->addAction(actions[LC_FILE_NEW]);
	standardToolBar->addAction(actions[LC_FILE_OPEN]);
	standardToolBar->addAction(actions[LC_FILE_SAVE]);
	standardToolBar->addAction(actions[LC_FILE_PRINT]);
	standardToolBar->addAction(actions[LC_FILE_PRINT_PREVIEW]);
	standardToolBar->addSeparator();
	standardToolBar->addAction(actions[LC_EDIT_UNDO]);
	standardToolBar->addAction(actions[LC_EDIT_REDO]);
	standardToolBar->addAction(actions[LC_EDIT_CUT]);
	standardToolBar->addAction(actions[LC_EDIT_COPY]);
	standardToolBar->addAction(actions[LC_EDIT_PASTE]);
	standardToolBar->addSeparator();
	standardToolBar->addAction(actions[LC_EDIT_LOCK_TOGGLE]);
	standardToolBar->addAction(actions[LC_EDIT_SNAP_TOGGLE]);
	standardToolBar->addAction(actions[LC_EDIT_SNAP_ANGLE]);
	standardToolBar->addSeparator();
	standardToolBar->addAction(actions[LC_EDIT_TRANSFORM]);

	QHBoxLayout *transformLayout = new QHBoxLayout;
	QWidget *transformWidget = new QWidget();
	transformWidget->setLayout(transformLayout);
	transformX = new QLineEdit();
	transformX->setMaximumWidth(75);
	transformLayout->addWidget(transformX);
	transformY = new QLineEdit();
	transformY->setMaximumWidth(75);
	transformLayout->addWidget(transformY);
	transformZ = new QLineEdit();
	transformZ->setMaximumWidth(75);
	transformLayout->addWidget(transformZ);
	transformLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	standardToolBar->addWidget(transformWidget);
	connect(transformX, SIGNAL(returnPressed()), actions[LC_EDIT_TRANSFORM], SIGNAL(triggered()));
	connect(transformY, SIGNAL(returnPressed()), actions[LC_EDIT_TRANSFORM], SIGNAL(triggered()));
	connect(transformZ, SIGNAL(returnPressed()), actions[LC_EDIT_TRANSFORM], SIGNAL(triggered()));

	toolsToolBar = addToolBar(tr("Tools"));
	toolsToolBar->setObjectName("ToolsToolbar");
	insertToolBarBreak(toolsToolBar);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_INSERT]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_LIGHT]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_SPOTLIGHT]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_CAMERA]);
	toolsToolBar->addSeparator();
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_SELECT]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_MOVE]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_ROTATE]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_DELETE]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_PAINT]);
	toolsToolBar->addSeparator();
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_ZOOM]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_PAN]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_ROTATE_VIEW]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_ROLL]);
	toolsToolBar->addAction(actions[LC_EDIT_ACTION_ZOOM_REGION]);

	timeToolBar = addToolBar(tr("Time"));
	timeToolBar->setObjectName("TimeToolbar");
	timeToolBar->addAction(actions[LC_VIEW_TIME_FIRST]);
	timeToolBar->addAction(actions[LC_VIEW_TIME_PREVIOUS]);
	timeToolBar->addAction(actions[LC_VIEW_TIME_NEXT]);
	timeToolBar->addAction(actions[LC_VIEW_TIME_LAST]);
	//LC_VIEW_TIME_STOP
	//LC_VIEW_TIME_PLAY
	//LC_VIEW_TIME_ANIMATION
	//LC_VIEW_TIME_ADD_KEYS

	partsToolBar = new QDockWidget(tr("Parts"), this);
	partsToolBar->setObjectName("PartsToolbar");
	partsToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	QWidget *partsContents = new QWidget();
	QGridLayout *partsLayout = new QGridLayout(partsContents);
	partsLayout->setSpacing(6);
	partsLayout->setContentsMargins(6, 6, 6, 6);
	QSplitter *partsSplitter = new QSplitter(Qt::Vertical, partsContents);

	QFrame *previewFrame = new QFrame(partsSplitter);
	previewFrame->setFrameShape(QFrame::StyledPanel);
	previewFrame->setFrameShadow(QFrame::Sunken);

	QGridLayout *previewLayout = new QGridLayout(previewFrame);
	previewLayout->setContentsMargins(0, 0, 0, 0);

	int AASamples = lcGetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES);
	if (AASamples > 1)
	{
		QGLFormat format;
		format.setSampleBuffers(true);
		format.setSamples(AASamples);
		QGLFormat::setDefaultFormat(format);
	}

	piecePreview = new lcGLWidget(previewFrame, NULL, new PiecePreview(), false);
	piecePreview->preferredSize = QSize(200, 100);
	previewLayout->addWidget(piecePreview, 0, 0, 1, 1);

	partsTree = new lcQPartsTree(partsSplitter);
	partsTree->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	connect(partsTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(partsTreeItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	partSearch = new QLineEdit(partsSplitter);
	connect(partSearch, SIGNAL(returnPressed()), this, SLOT(partSearchReturn()));
	connect(partSearch, SIGNAL(textChanged(QString)), this, SLOT(partSearchChanged(QString)));

	QCompleter *completer = new QCompleter(new lcQPartsListModel(), this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	partSearch->setCompleter(completer);

	QFrame *colorFrame = new QFrame(partsSplitter);
	colorFrame->setFrameShape(QFrame::StyledPanel);
	colorFrame->setFrameShadow(QFrame::Sunken);

	QGridLayout *colorLayout = new QGridLayout(colorFrame);
	colorLayout->setContentsMargins(0, 0, 0, 0);

	colorList = new lcColorListWidget(partsSplitter);
	colorLayout->addWidget(colorList);
	connect(colorList, SIGNAL(colorChanged(int)), this, SLOT(colorChanged(int)));

	partsLayout->addWidget(partsSplitter, 0, 0, 1, 1);

	partsToolBar->setWidget(partsContents);
	addDockWidget(Qt::RightDockWidgetArea, partsToolBar);

	propertiesToolBar = new QDockWidget(tr("Properties"), this);
	propertiesToolBar->setObjectName("PropertiesToolbar");
	propertiesToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	QWidget *propertiesContents = new QWidget();
	QFormLayout *propertiesLayout = new QFormLayout(propertiesContents);
	propertiesLayout->setSpacing(6);
	propertiesLayout->setContentsMargins(6, 6, 6, 6);

	QLabel *label = new QLabel(tr("Nothing selected"), propertiesContents);
	propertiesLayout->addWidget(label);

	propertiesToolBar->setWidget(propertiesContents);
	addDockWidget(Qt::RightDockWidgetArea, propertiesToolBar);

	tabifyDockWidget(partsToolBar, propertiesToolBar);
	partsToolBar->raise();
}

void lcQMainWindow::createStatusBar()
{
	statusBar = new QStatusBar(this);
	setStatusBar(statusBar);

	statusPositionLabel = new QLabel("XYZ");
	statusBar->addPermanentWidget(statusPositionLabel);

	statusSnapLabel = new QLabel();
	statusBar->addPermanentWidget(statusSnapLabel);

	statusTimeLabel = new QLabel();
	statusBar->addPermanentWidget(statusTimeLabel);
}

void lcQMainWindow::closeEvent(QCloseEvent *event)
{
	if (!lcGetActiveProject()->SaveModified())
		event->ignore();
	else
		event->accept();
}

QMenu *lcQMainWindow::createPopupMenu()
{
    QMenu *menuToolBars = new QMenu(this);

	menuToolBars->addAction(partsToolBar->toggleViewAction());
	menuToolBars->addAction(propertiesToolBar->toggleViewAction());
	menuToolBars->addSeparator();
	menuToolBars->addAction(standardToolBar->toggleViewAction());
	menuToolBars->addAction(toolsToolBar->toggleViewAction());
	menuToolBars->addAction(timeToolBar->toggleViewAction());

	return menuToolBars;
}

void lcQMainWindow::actionTriggered()
{
	QObject *action = sender();

	for (int Command = 0; Command < LC_NUM_COMMANDS; Command++)
	{
		if (action == actions[Command])
		{
			lcGetActiveProject()->HandleCommand((LC_COMMANDS)Command);
			break;
		}
	}
}

void lcQMainWindow::partsTreeItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	PieceInfo *info = (PieceInfo*)current->data(0, lcQPartsTree::PartInfoRole).value<void*>();

	if (info)
	{
		lcGetActiveProject()->SetCurrentPiece(info);
		PiecePreview* preview = (PiecePreview*)piecePreview->mWindow;
		preview->OnInitialUpdate();
		preview->SetCurrentPiece(info);
	}
}

void lcQMainWindow::colorChanged(int colorIndex)
{
	lcGetActiveProject()->HandleNotify(LC_COLOR_CHANGED, colorIndex);

	PiecePreview* preview = (PiecePreview*)piecePreview->mWindow;
	preview->Redraw();
}

void lcQMainWindow::partSearchReturn()
{
	partsTree->searchParts(partSearch->text());
}

void lcQMainWindow::partSearchChanged(const QString& text)
{
	const char *searchString = text.toLocal8Bit().data();
	int length = strlen(searchString);

	if (!length)
		return;

	lcPiecesLibrary *library = lcGetPiecesLibrary();
	PieceInfo* bestMatch = NULL;

	for (int partIndex = 0; partIndex < library->mPieces.GetSize(); partIndex++)
	{
		PieceInfo *info = library->mPieces[partIndex];

		if (strncasecmp(searchString, info->m_strDescription, length) == 0)
		{
			if (!bestMatch || strcasecmp(bestMatch->m_strDescription, info->m_strDescription) > 0)
				bestMatch = info;
		}
		else if (strncasecmp(searchString, info->m_strName, length) == 0)
		{
			if (!bestMatch || strcasecmp(bestMatch->m_strName, info->m_strName) > 0)
				bestMatch = info;
		}
	}

	if (bestMatch)
		partsTree->setCurrentPart(bestMatch);
}

void lcQMainWindow::clipboardChanged()
{
	const QString mimeType("application/vnd.leocad-clipboard");
	const QMimeData *mimeData = QApplication::clipboard()->mimeData();
	lcMemFile *clipboard = NULL;

	if (mimeData->hasFormat(mimeType))
	{
		QByteArray clipboardData = mimeData->data(mimeType);

		clipboard = new lcMemFile();
		clipboard->WriteBuffer(clipboardData.constData(), clipboardData.size());
	}

	g_App->SetClipboard(clipboard);
}

void lcQMainWindow::splitView(Qt::Orientation orientation)
{
	QWidget *focus = focusWidget();

	if (typeid(*focus) != typeid(lcGLWidget))
		return;

	QWidget *parent = focus->parentWidget();
	QSplitter *splitter;
	QList<int> sizes;

	if (parent == centralWidget())
	{
		splitter = new QSplitter(orientation, parent);
		parent->layout()->addWidget(splitter);
		splitter->addWidget(focus);
		splitter->addWidget(new lcGLWidget(centralWidget(), piecePreview, new View(lcGetActiveProject()), true));
	}
	else
	{
		QSplitter *parentSplitter = (QSplitter*)parent;	
		sizes = parentSplitter->sizes();
		int focusIndex = parentSplitter->indexOf(focus);

		splitter = new QSplitter(orientation, parent);
		parentSplitter->insertWidget(focusIndex, splitter);
		splitter->addWidget(focus);
		splitter->addWidget(new lcGLWidget(centralWidget(), piecePreview, new View(lcGetActiveProject()), true));

		parentSplitter->setSizes(sizes);
	}

	sizes.clear();
	sizes.append(10);
	sizes.append(10);
	splitter->setSizes(sizes);
}

void lcQMainWindow::splitHorizontal()
{
	splitView(Qt::Vertical);
}

void lcQMainWindow::splitVertical()
{
	splitView(Qt::Horizontal);
}

void lcQMainWindow::removeView()
{
	QWidget *focus = focusWidget();

	if (typeid(*focus) != typeid(lcGLWidget))
		return;

	QWidget *parent = focus->parentWidget();

	if (parent == centralWidget())
		return;

	QWidget *parentParentWidget = parent->parentWidget();
	QSplitter *parentSplitter = (QSplitter*)parent;
	int focusIndex = parentSplitter->indexOf(focus);

	if (parentParentWidget == centralWidget())
	{
		QLayout* centralLayout = parentParentWidget->layout();

		centralLayout->addWidget(parentSplitter->widget(!focusIndex));
		centralLayout->removeWidget(parent);

		return;
	}

	QSplitter* parentParentSplitter = (QSplitter*)parentParentWidget;
	QList<int> sizes = parentParentSplitter->sizes();

	int parentIndex = parentParentSplitter->indexOf(parent);
	parentParentSplitter->insertWidget(!parentIndex, focus);

	delete parent;

	parentParentSplitter->setSizes(sizes);
}

void lcQMainWindow::resetViews()
{
	QLayout* centralLayout = centralWidget()->layout();
	delete centralLayout->itemAt(0)->widget();
	centralLayout->addWidget(new lcGLWidget(centralWidget(), piecePreview, new View(lcGetActiveProject()), true));
}

void lcQMainWindow::toggleFullScreen()
{
	// todo: hide toolbars and menu
	// todo: create fullscreen toolbar or support esc key to go back
	if (isFullScreen())
		showNormal();
	else
		showFullScreen();

}

void lcQMainWindow::updateAction(int newAction)
{
	QAction *action = actions[LC_EDIT_ACTION_FIRST + newAction];

	if (action)
		action->setChecked(true);
}

void lcQMainWindow::updatePaste(bool enabled)
{
	QAction *action = actions[LC_EDIT_PASTE];

	if (action)
		action->setEnabled(enabled);
}

void lcQMainWindow::updateTime(bool animation, int currentTime, int totalTime)
{
	actions[LC_VIEW_TIME_FIRST]->setEnabled(currentTime != 1);
	actions[LC_VIEW_TIME_PREVIOUS]->setEnabled(currentTime > 1);
	actions[LC_VIEW_TIME_NEXT]->setEnabled(currentTime < totalTime);
	actions[LC_VIEW_TIME_LAST]->setEnabled(currentTime != totalTime);

	if (animation)
		statusTimeLabel->setText(QString(tr(" %1 / %2 ")).arg(QString::number(currentTime), QString::number(totalTime)));
	else
		statusTimeLabel->setText(QString(tr(" Step %1 ")).arg(QString::number(currentTime)));
}

void lcQMainWindow::updateAnimation(bool animation, bool addKeys)
{
	/*
	gtk_widget_set_sensitive (anim_toolbar.play, bAnimation);
	gtk_widget_set_sensitive (anim_toolbar.stop, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(anim_toolbar.anim), bAnimation);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(anim_toolbar.keys), bAddKeys);
	gpointer item = gtk_object_get_data (GTK_OBJECT (((GtkWidget*)(*main_window))), "menu_piece_copykeys");
	gtk_label_set_text (GTK_LABEL (GTK_BIN (item)->child), bAnimation ? "Copy Keys from Instructions" : "Copy Keys from Animation");
	*/
}

void lcQMainWindow::updateLockSnap(lcuint32 snap)
{
	actions[LC_EDIT_SNAP_X]->setChecked((snap & LC_DRAW_SNAP_X) != 0);
	actions[LC_EDIT_SNAP_Y]->setChecked((snap & LC_DRAW_SNAP_Y) != 0);
	actions[LC_EDIT_SNAP_Z]->setChecked((snap & LC_DRAW_SNAP_Z) != 0);

	actions[LC_EDIT_LOCK_X]->setChecked((snap & LC_DRAW_LOCK_X) != 0);
	actions[LC_EDIT_LOCK_Y]->setChecked((snap & LC_DRAW_LOCK_Y) != 0);
	actions[LC_EDIT_LOCK_Z]->setChecked((snap & LC_DRAW_LOCK_Z) != 0);
}

void lcQMainWindow::updateSnap()
{
	char xy[32], z[32], angle[32];
	int moveXYSnapIndex, moveZSnapIndex, angleSnapIndex;
	Project *project = lcGetActiveProject();

	project->GetSnapText(xy, z, angle);
	project->GetSnapIndex(&moveXYSnapIndex, &moveZSnapIndex, &angleSnapIndex);

	actions[LC_EDIT_SNAP_MOVE_XY0 + moveXYSnapIndex]->setChecked(true);
	actions[LC_EDIT_SNAP_MOVE_Z0 + moveZSnapIndex]->setChecked(true);
	actions[LC_EDIT_SNAP_ANGLE0 + angleSnapIndex]->setChecked(true);

	statusSnapLabel->setText(QString(tr(" M: %1 %2 R: %3 ")).arg(xy, z, angle));
}

void lcQMainWindow::updateUndoRedo(const char* undoText, const char* redoText)
{
	QAction *undoAction = actions[LC_EDIT_UNDO];
	QAction *redoAction = actions[LC_EDIT_REDO];

	if (undoText)
	{
		undoAction->setEnabled(true);
		undoAction->setText(QString(tr("Undo %1")).arg(undoText));
	}
	else
	{
		undoAction->setEnabled(false);
		undoAction->setText(tr("Undo"));
	}

	if (redoText)
	{
		redoAction->setEnabled(true);
		redoAction->setText(QString(tr("Redo %1")).arg(redoText));
	}
	else
	{
		redoAction->setEnabled(false);
		redoAction->setText(tr("Redo"));
	}
}

void lcQMainWindow::updateTransformType(int newType)
{
	actions[LC_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION + newType]->setChecked(true);
}

void lcQMainWindow::updateCameraMenu(const PtrArray<Camera>& cameras, Camera* currentCamera)
{
	int actionIdx, currentIndex = -1;

	for (actionIdx = LC_VIEW_CAMERA_FIRST; actionIdx <= LC_VIEW_CAMERA_LAST; actionIdx++)
	{
		QAction* action = actions[actionIdx];
		int cameraIdx = actionIdx - LC_VIEW_CAMERA_FIRST;

		if (cameraIdx < cameras.GetSize())
		{
			if (currentCamera == cameras[cameraIdx])
				currentIndex = cameraIdx;

			action->setText(cameras[cameraIdx]->GetName());
			action->setVisible(true);
		}
		else
			action->setVisible(false);
	}

	updateCurrentCamera(currentIndex);
}

void lcQMainWindow::updateCurrentCamera(int cameraIndex)
{
	int actionIndex = LC_VIEW_CAMERA_FIRST + cameraIndex;

	if (actionIndex < LC_VIEW_CAMERA_FIRST || actionIndex > LC_VIEW_CAMERA_LAST)
		actionIndex = LC_VIEW_CAMERA_NONE;

	actions[actionIndex]->setChecked(true);
}

void lcQMainWindow::updateCategories()
{
	partsTree->updateCategories();
}

void lcQMainWindow::updateTitle(const char* title, bool modified)
{
	setWindowModified(modified);
	setWindowFilePath(title);
}

void lcQMainWindow::updateModified(bool modified)
{
	setWindowModified(modified);
}

void lcQMainWindow::updateRecentFiles(const char** fileNames)
{
	for (int actionIdx = LC_FILE_RECENT_FIRST; actionIdx <= LC_FILE_RECENT_LAST; actionIdx++)
	{
		int fileIdx = actionIdx - LC_FILE_RECENT_FIRST;
		QAction *action = actions[actionIdx];

		if (fileNames[fileIdx][0])
		{
			action->setText(QString("&%1 %2").arg(QString::number(fileIdx + 1), QDir::toNativeSeparators(fileNames[fileIdx])));
			action->setVisible(true);
		}
		else
			action->setVisible(false);
	}

	actionFileRecentSeparator->setVisible(fileNames[0][0] != 0);
}

void lcQMainWindow::updateShortcuts()
{
	for (int actionIdx = 0; actionIdx < LC_NUM_COMMANDS; actionIdx++)
		actions[actionIdx]->setShortcut(QKeySequence(gKeyboardShortcuts.Shortcuts[actionIdx]));
}

lcVector3 lcQMainWindow::getTransformAmount()
{
	lcVector3 transform;

	transform.x = transformX->text().toFloat();
	transform.y = transformY->text().toFloat();
	transform.z = transformZ->text().toFloat();

	return transform;
}
