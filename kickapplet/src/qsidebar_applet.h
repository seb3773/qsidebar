#ifndef QSIDEBAR_APPLET_H
#define QSIDEBAR_APPLET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqstring.h>
#include <tqlabel.h>
#include <tqwidget.h>
#include <tqtimer.h>
#include <tqpopupmenu.h>
#include <kpanelapplet.h>
#include <dcopobject.h>

class TQLabel;

class qsidebar_applet : public KPanelApplet, public DCOPObject
{
    TQ_OBJECT
    K_DCOP

public:
    qsidebar_applet(const TQString& configFile, Type t = Normal, int actions = 0, TQWidget *parent = 0, const char *name = 0);
    ~qsidebar_applet();

k_dcop:
    bool setIconPath(const TQString &iconPath, bool enable, bool focus, bool appicons, bool notifcount);
    virtual int widthForHeight(int height) const;
    virtual int heightForWidth(int width) const;

protected:
    void resizeEvent(TQResizeEvent *);
    bool eventFilter(TQObject *obj, TQEvent *event);
    void loadIcon(const TQString &iconPath, bool enable);

private slots:
    void iconClicked();
    void checkDesktopReady();
    void sendSignalToQSidebar();
    void showContextMenu(const TQPoint &globalPos);

private:
    TQWidget *mainView;
    TQLabel *m_iconLabel;
    TQString m_currentIconPath;
    pid_t m_qsidebarPid;
    pid_t findQSidebarPid();
    pid_t launchQSidebar();
    bool isPidValid(pid_t pid);

    // États pour les options cochables du menu
    bool m_focusAssistEnabled;
    bool m_hideAppIcons;
    bool m_hideNotifCount;
};

#endif
