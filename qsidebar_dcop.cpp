#include <dcopclient.h>
#include <tqcstring.h>
#include <tqbuffer.h>
#include <tqfile.h>
#include "qsidebar_dcop.h"

bool setSidebarIcon(const TQString& iconPath, bool enable, bool focus, bool appicons, bool notifcount)
{
    DCOPClient *client = new DCOPClient();
    if (!client->attach()) {
        delete client;
        return false;
    }

    TQBuffer buffer;
    buffer.open(IO_WriteOnly);
    TQDataStream arg(&buffer);

    arg << iconPath << enable << focus << appicons << notifcount;

    buffer.close();
    TQByteArray data = buffer.buffer();

    bool result = client->send("kicker", "QSidebarAppletIface",
        "setIconPath(TQString,bool,bool,bool,bool)", data);

    delete client;
    return result;
}

extern "C" {
int set_sidebar_icon(const char* icon_path, int enable, int focus, int appicons, int notifcount)
{
    TQString path = TQString::fromUtf8(icon_path);
    bool result = setSidebarIcon(path, enable != 0, focus != 0, appicons != 0, notifcount != 0);
    return result ? 1 : 0;
}
}