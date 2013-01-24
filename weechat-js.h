#ifndef __WEECHAT_JS_H_
#define __WEECHAT_JS_H_

#include "weechat-plugin.h"

#ifdef __cplusplus
#ifdef _WIN32
#define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT extern "C"
#endif
#else
#define EXPORT
#endif

#define weechat_plugin weechat_js_plugin
#define JS_PLUGIN_NAME "js"

#define JS_CURRENT_SCRIPT_NAME ((js_current_script) ? js_current_script->name : "-")

extern struct t_weechat_plugin *weechat_js_plugin;

extern int js_quiet;
extern struct t_plugin_script *js_scripts;
extern struct t_plugin_script *last_js_script;
extern struct t_plugin_script *js_current_script;
extern struct t_plugin_script *js_registered_script;
extern const char *js_current_script_filename;

#endif /* __WEECHAT_JS_H_ */
