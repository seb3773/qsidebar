#include <tdeapplication.h>
#include <tqlabel.h>
#include <tqpixmap.h>
#include <tqbitmap.h>
#include <tqimage.h>
#include <tqevent.h>
#include <tqpopupmenu.h>
#include <tqtimer.h>
#include <tqstring.h>
#include <tqwidget.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qsidebar_applet.h"
#include <dcopclient.h>

qsidebar_applet::qsidebar_applet(const TQString& configFile, Type type, int actions, TQWidget *parent, const char *name)
    : KPanelApplet(configFile, type, actions, parent, name), DCOPObject("QSidebarAppletIface"),
      m_focusAssistEnabled(false), m_hideAppIcons(false), m_hideNotifCount(false)
{
    setFixedSize(42, 42);

    m_iconLabel = new TQLabel(this);
    m_iconLabel->setBackgroundOrigin(AncestorOrigin);
    m_iconLabel->setGeometry(0, 0, 42, 42);
    m_iconLabel->setAlignment(TQt::AlignCenter);

    TQPixmap placeholder(42, 42);
    TQBitmap mask(42, 42);
    mask.fill(TQt::color0);
    placeholder.setMask(mask);
    m_iconLabel->setPixmap(placeholder);

    m_iconLabel->installEventFilter(this);

    mainView = m_iconLabel;
    mainView->show();

    if (!kapp->dcopClient()->isAttached()) {
        kapp->dcopClient()->attach();
    }
    kapp->dcopClient()->setDefaultObject(objId());

    m_qsidebarPid = findQSidebarPid();
    if (m_qsidebarPid == -1) {
        TQTimer::singleShot(1500, this, SLOT(checkDesktopReady()));
    } else {
        kill(m_qsidebarPid, SIGWINCH);
    }
}

qsidebar_applet::~qsidebar_applet()
{
}

void qsidebar_applet::checkDesktopReady()
{
    if (kapp->dcopClient()->isApplicationRegistered("kicker")) {
        m_qsidebarPid = launchQSidebar();
    } else {
        TQTimer::singleShot(1000, this, SLOT(checkDesktopReady()));
    }
}

void qsidebar_applet::loadIcon(const TQString &iconPath, bool enable)
{
    if (iconPath == m_currentIconPath) {
        return;
    }
    TQPixmap originalIcon(iconPath);
    if (!originalIcon.isNull()) {
        TQImage image = originalIcon.convertToImage();
        image = image.convertDepth(32);
        if (enable) {
            image.invertPixels();
        }
        TQPixmap scaledIcon = TQPixmap(image.smoothScale(42, 42));
        m_iconLabel->setPixmap(scaledIcon);
        m_currentIconPath = iconPath;
    }
}

bool qsidebar_applet::setIconPath(const TQString &iconPath, bool enable, bool focus, bool appicons, bool notifcount)
{
    m_focusAssistEnabled = focus;
    m_hideAppIcons = appicons;
    m_hideNotifCount = notifcount;
    bool result = !iconPath.isEmpty();
    if (result) {
        loadIcon(iconPath, enable);
    }
    return result;
}


bool qsidebar_applet::eventFilter(TQObject *obj, TQEvent *event)
{
    if (!kapp->authorizeTDEAction("kicker_rmb"))
        return false;
    if (obj == m_iconLabel) {
        if (event->type() == TQEvent::MouseButtonRelease) {
            TQMouseEvent *mouseEvent = static_cast<TQMouseEvent *>(event);
            if (mouseEvent->button() == TQt::LeftButton) {
                iconClicked();
                return true;
            }
        }
        else if (event->type() == TQEvent::MouseButtonPress) {
            TQMouseEvent *mouseEvent = static_cast<TQMouseEvent *>(event);
            if (mouseEvent->button() == TQt::RightButton) {
                if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0)) {
                    kill(m_qsidebarPid, SIGUSR2);
                }
                showContextMenu(mouseEvent->globalPos());
                return true;
            }
        }
    }
    return KPanelApplet::eventFilter(obj, event);
}


void qsidebar_applet::showContextMenu(const TQPoint &globalPos)
{
    TQPopupMenu *menu = new TQPopupMenu(this);
    int openSidebarId = menu->insertItem(tr("  Open sidebar                                                  "));
    menu->insertSeparator();
    int focusAssistId = menu->insertItem(tr("  Focus assist"));
    menu->insertSeparator();
    int noIconsId = menu->insertItem(tr("  Don't display applications icons"));
    int noNotifId = menu->insertItem(tr("  Don't display notifications count"));
    menu->setCheckable(true);
    menu->setItemChecked(focusAssistId, m_focusAssistEnabled);
    menu->setItemChecked(noIconsId, m_hideAppIcons);
    menu->setItemChecked(noNotifId, m_hideNotifCount);
    int id = menu->exec(globalPos);
    if (id == openSidebarId) {
        sendSignalToQSidebar();
    } else if (id == focusAssistId) {
        m_focusAssistEnabled = !m_focusAssistEnabled;
            if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0))
        kill(m_qsidebarPid, SIGALRM);
    } else if (id == noIconsId) {
        m_hideAppIcons = !m_hideAppIcons;
    if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0))
        kill(m_qsidebarPid, SIGXFSZ);
    } else if (id == noNotifId) {
        m_hideNotifCount = !m_hideNotifCount;
          if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0))
        kill(m_qsidebarPid, SIGVTALRM);
    }

    delete menu;
}


pid_t qsidebar_applet::findQSidebarPid()
{
    DIR *dir = opendir("/proc");
    if (!dir) return -1;
    struct dirent *entry;
    char path[256], comm[256];
    pid_t pid = -1;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR) continue;
        int id = atoi(entry->d_name);
        if (id <= 0) continue;
        snprintf(path, sizeof(path), "/proc/%d/comm", id);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        if (fgets(comm, sizeof(comm), f)) {
            if (strncmp(comm, "qsidebar", 8) == 0) {
                pid = id;
                fclose(f);
                break;
            }
        }
        fclose(f);
    }
    closedir(dir);
    return pid;
}

pid_t qsidebar_applet::launchQSidebar()
{
struct sigaction sa;
sa.sa_handler = SIG_IGN;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_NOCLDWAIT;
sigaction(SIGCHLD, &sa, 0);
    pid_t pid = fork();
    if (pid == 0) {
        execl("/usr/local/bin/qsidebar", "qsidebar", (char*)0);
        _exit(127);
    }
    return pid > 0 ? pid : -1;
}

bool qsidebar_applet::isPidValid(pid_t pid)
{
    return pid > 0 && !kill(pid, 0);
}

void qsidebar_applet::iconClicked()
{
    if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0) && !kill(m_qsidebarPid, SIGUSR1)) return;
    m_qsidebarPid = findQSidebarPid();
    if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0) && !kill(m_qsidebarPid, SIGUSR1)) return;
    m_qsidebarPid = launchQSidebar();
    if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0))
        TQTimer::singleShot(500, this, SLOT(sendSignalToQSidebar()));
}

void qsidebar_applet::sendSignalToQSidebar()
{
    if (m_qsidebarPid > 0 && !kill(m_qsidebarPid, 0))
        kill(m_qsidebarPid, SIGUSR1);
}

int qsidebar_applet::widthForHeight(int height) const
{
    return width();
}

int qsidebar_applet::heightForWidth(int width) const
{
    return height();
}

void qsidebar_applet::resizeEvent(TQResizeEvent *e)
{
    updateGeometry();
}

extern "C" {
KPanelApplet* init(TQWidget *parent, const TQString& configFile)
{
    // TDEGlobal::locale()->insertCatalogue("qsidebar_applet");
    return new qsidebar_applet(configFile, KPanelApplet::Normal, 0, parent, "qsidebar_applet");
}
}
