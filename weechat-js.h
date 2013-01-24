/*
 * Copyright (C) 2013 Koka El Kiwi <admin@kokabsolu.com>
 *
 * This file is part of WeeChat, the extensible chat client.
 *
 * WeeChat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * WeeChat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WeeChat.  If not, see <http://www.gnu.org/licenses/>.
 */
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
