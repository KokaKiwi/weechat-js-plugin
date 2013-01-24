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
#include <cstdlib>
#include <cstdio>
#include <cstring>

extern "C"
{
#include "weechat-plugin.h"
#include "plugin-script.h"
#include "weechat-js.h"
}

#include "weechat-js-core.h"

WEECHAT_PLUGIN_NAME(JS_PLUGIN_NAME);
WEECHAT_PLUGIN_DESCRIPTION("Support of js scripts");
WEECHAT_PLUGIN_AUTHOR("Koka El Kiwi <admin@kokabsolu.com>");
WEECHAT_PLUGIN_VERSION("0.1.0");
WEECHAT_PLUGIN_LICENSE("GPL3");

struct t_weechat_plugin *weechat_js_plugin;

int js_quiet = 0;
struct t_plugin_script *js_scripts = NULL;
struct t_plugin_script *last_js_script = NULL;
struct t_plugin_script *js_current_script = NULL;
struct t_plugin_script *js_registered_script = NULL;
const char *js_current_script_filename = NULL;

/*
 * Load a js script.
 */

int
weechat_js_load (const char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL)
    {
        weechat_printf(NULL,
                       weechat_gettext("%s%s: script \"%s\" not found"),
                       weechat_prefix("error"), JS_PLUGIN_NAME, filename);
        return 0;
    }

    if ((weechat_js_plugin->debug >= 2) || !js_quiet)
    {
        weechat_printf(NULL,
                       weechat_gettext("%s: loading script \"%s\""),
                       JS_PLUGIN_NAME, filename);
    }

    js_current_script = NULL;
    js_registered_script = NULL;

    js_current_core = new WeechatJsCore();

    if (js_current_core == NULL)
    {
        weechat_printf(NULL,
                       weechat_gettext("%s%s: unable to create new"
                                       "sub-interpreter"),
                       weechat_prefix("error"), JS_PLUGIN_NAME);
        fclose(fp);
        return 0;
    }

    // Load libs
    js_current_core->loadLibs();

    js_current_script_filename = filename;

    if (!js_current_core->loadFile(fp))
    {
        weechat_printf(NULL,
                       weechat_gettext ("%s%s: unable to load file \"%s\""),
                       weechat_prefix("error"), JS_PLUGIN_NAME);
        delete js_current_core;
        fclose(fp);

        /* if script was registered, remove it from list */
        if (js_current_script)
        {
            plugin_script_remove (weechat_js_plugin, &js_scripts, &last_js_script,
                                  js_current_script);
        }

        return 0;
    }

    if (!js_current_core->execute())
    {
        weechat_printf (NULL,
                        weechat_gettext("%s%s: unable to execute file "
                                         "\"%s\""),
                        weechat_prefix("error"), JS_PLUGIN_NAME, filename);
        delete js_current_core;
        fclose(fp);
        return 0;
    }

    fclose(fp);

    if (!js_registered_script)
    {
        weechat_printf(NULL,
                       weechat_gettext("%s%s: function \"register\" not found"
                                       " (or failed) in file \"%s\""),
                       weechat_prefix("error"), JS_PLUGIN_NAME, filename);
        delete js_current_core;
        return 0;
    }

    js_current_script = js_registered_script;

    js_current_script->interpreter = js_current_core;

    weechat_hook_signal_send ("lua_script_loaded", WEECHAT_HOOK_SIGNAL_STRING,
                              js_current_script->filename);

    return 1;
}

/*
 * Callback called when loading a file.
 */
void
weechat_js_load_cb (void *data, const char *filename)
{
    weechat_js_load(filename);
}

void
weechat_js_unload (struct t_plugin_script *script)
{
    char *filename;
    void *interpreter;

    if ((weechat_js_plugin->debug >= 2) || !js_quiet)
    {
        weechat_printf(NULL, weechat_gettext("%s: unloading script \"%s\""),
                       JS_PLUGIN_NAME, script->name);
    }

    filename = strdup(script->filename);
    interpreter = script->interpreter;

    if (js_current_script == script)
        js_current_script = (js_current_script->prev_script) ?
            js_current_script->prev_script : js_current_script->next_script;

    plugin_script_remove(weechat_js_plugin, &js_scripts,
                         &last_js_script, script);

    if (interpreter)
        delete ((WeechatJsCore *) interpreter);

    weechat_hook_signal_send("js_script_unloaded", WEECHAT_HOOK_SIGNAL_STRING,
                             filename);

    if (filename)
        free(filename);
}

/*
 * Unload all js scripts.
 */

void
weechat_js_unload_all ()
{
    while (js_scripts)
    {
        weechat_js_unload(js_scripts);
    }
}

/*
 * Callback for command "/js"
 */

int
weechat_js_command_cb (void *data, struct t_gui_buffer *buffer,
                       int argc, char **argv, char **argv_eol)
{
    char *ptr_name, *path_script;

    if (argc == 1)
    {
        plugin_script_display_list(weechat_js_plugin, js_scripts,
                                   NULL, 0);
    }
    else if (argc == 2)
    {
        if (weechat_strcasecmp(argv[1], "list") == 0)
        {
            plugin_script_display_list(weechat_js_plugin, js_scripts,
                                       NULL, 0);
        }
        else if (weechat_strcasecmp(argv[1], "listfull") == 0)
        {
            plugin_script_display_list(weechat_js_plugin, js_scripts,
                                       NULL, 1);
        }
        else if (weechat_strcasecmp(argv[1], "unload"))
        {
            weechat_js_unload_all();
        }
    }
    else if (argc == 3)
    {
        if (weechat_strcasecmp(argv[1], "load") == 0)
        {
            ptr_name = argv_eol[2];
            if (strncmp (ptr_name, "-q ", 3) == 0)
            {
                js_quiet = 1;
                ptr_name += 3;
                while (ptr_name[0] == ' ')
                {
                    ptr_name++;
                }
            }
            if (weechat_strcasecmp (argv[1], "load") == 0)
            {
                /* load lua script */
                path_script = plugin_script_search_path (weechat_js_plugin,
                                                         ptr_name);
                weechat_js_load ((path_script) ? path_script : ptr_name);
                if (path_script)
                    free (path_script);
            }
        }
    }

    return WEECHAT_RC_OK;
}

/*
 * Adds js scripts to completion list.
 */

int
weechat_js_completion_cb (void *data, const char *completion_item,
                          struct t_gui_buffer *buffer,
                          struct t_gui_completion *completion)
{
    plugin_script_completion(weechat_js_plugin, completion, js_scripts);

    return WEECHAT_RC_OK;
}

/*
 * Returns hdata for js scripts.
 */

struct t_hdata *
weechat_js_hdata_cb (void *data, const char *hdata_name)
{
    return plugin_script_hdata_script(weechat_plugin,
                                      &js_scripts, &last_js_script,
                                      hdata_name);
}

/*
 * Returns infolist with js scripts.
 */

struct t_infolist *
weechat_js_infolist_cb (void *data, const char *infolist_name,
                        void *pointer, const char *arguments)
{
    if (!infolist_name || !infolist_name[0])
        return NULL;

    if (weechat_strcasecmp(infolist_name, "js_script") == 0)
    {
        return plugin_script_infolist_list_scripts(weechat_js_plugin,
                                                   js_scripts, pointer,
                                                   arguments);
    }

    return NULL;
}

/*
 * Dumps js plugin data in Weechat log file.
 */

int
weechat_js_signal_debug_dump_cb (void *data, const char *signal,
                                 const char *type_data, void *signal_data)
{
    if (!signal_data
        || (weechat_strcasecmp((char *) signal_data, JS_PLUGIN_NAME) == 0))
    {
        plugin_script_print_log(weechat_js_plugin, js_scripts);
    }

    return WEECHAT_RC_OK;
}

/*
 * Callback called when a buffer is closed.
 */

int
weechat_js_signal_buffer_closed_cb (void *data, const char *signal,
                                    const char *type_data, void *signal_data)
{
    if (signal_data)
        plugin_script_remove_buffer_callbacks(js_scripts,
                                              (struct t_gui_buffer *) signal_data);

    return WEECHAT_RC_OK;
}

/*
 * Timer for executing actions.
 */

int
weechat_js_timer_action_cb (void *data, int remaining_calls)
{
    return WEECHAT_RC_OK;
}

/*
 * Callback called when a script action is asked (install/remove a script).
 */

int
weechat_js_signal_script_action_cb (void *data, const char *signal,
                                    const char *type_data,
                                    void *signal_data)
{
    return WEECHAT_RC_OK;
}

/*
 * Initialize js plugin
 */

EXPORT int
weechat_plugin_init (struct t_weechat_plugin *plugin, int argc, char *argv[])
{
    struct t_plugin_script_init init;

    weechat_js_plugin = plugin;

    init.callback_command = &weechat_js_command_cb;
    init.callback_completion = &weechat_js_completion_cb;
    init.callback_hdata = &weechat_js_hdata_cb;
    init.callback_infolist = &weechat_js_infolist_cb;
    init.callback_signal_debug_dump = &weechat_js_signal_debug_dump_cb;
    init.callback_signal_buffer_closed = &weechat_js_signal_buffer_closed_cb;
    init.callback_signal_script_action = &weechat_js_signal_script_action_cb;
    init.callback_load_file = &weechat_js_load_cb;

    js_quiet = 1;
    plugin_script_init(plugin, argc, argv, &init);
    js_quiet = 0;

    plugin_script_display_short_list(weechat_js_plugin, js_scripts);

    return WEECHAT_RC_OK;
}

/*
 * Ends js plugin
 */

EXPORT int
weechat_plugin_end (struct t_weechat_plugin *plugin)
{
    js_quiet = 1;
    plugin_script_end(plugin, &js_scripts, &weechat_js_unload_all);
    js_quiet = 0;

    return WEECHAT_RC_OK;
}
