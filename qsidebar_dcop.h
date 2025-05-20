#ifndef QSIDEBAR_DCOP_H
#define QSIDEBAR_DCOP_H

#ifdef __cplusplus
extern "C" {
#endif

int set_sidebar_icon(const char* icon_path, int enable, int focus, int appicons, int notifcount);

#ifdef __cplusplus
}
#endif

#endif // QSIDEBAR_DCOP_H