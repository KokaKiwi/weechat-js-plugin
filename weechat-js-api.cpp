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
#undef _

#include <cstdlib>
#include <cstring>
#include <string>

extern "C"
{
#include "weechat-plugin.h"
#include "plugin-script.h"
#include "plugin-script-api.h"
#include "plugin-script-callback.h"
#include "weechat-js.h"
}

#include "weechat-js-core.h"
#include "weechat-js-api.h"

using namespace v8;

#define API_FUNC(__init, __name, __ret)                                 \
    std::string js_function_name(__name);                               \
    if (__init                                                          \
        && (!js_current_script || !js_current_script->name))            \
    {                                                                   \
        WEECHAT_SCRIPT_MSG_NOT_INIT(JS_CURRENT_SCRIPT_NAME,             \
                                    js_function_name.c_str());          \
        __ret;                                                          \
    }
#define API_WRONG_ARGS(__ret)                                           \
    {                                                                   \
        WEECHAT_SCRIPT_MSG_WRONG_ARGS(JS_CURRENT_SCRIPT_NAME,           \
                                      js_function_name.c_str());        \
        __ret;                                                          \
    }

#define API_PTR2STR(__pointer)                                          \
    plugin_script_ptr2str (__pointer)
#define API_STR2PTR(__string)                                           \
    plugin_script_str2ptr (weechat_js_plugin,                           \
                           JS_CURRENT_SCRIPT_NAME,                      \
                           js_function_name.c_str(), __string)

#define API_RETURN_OK return v8::True()
#define API_RETURN_ERROR return v8::False()

#define API_RETURN_STRING(__string)                                     \
    return String::New(__string)
#define API_RETURN_STRING_FREE(__string)                                \
    {                                                                   \
        Handle<Value> return_value = String::New(__string);             \
        free(__string);                                                 \
        return return_value;                                            \
    }
#define API_RETURN_EMPTY                                                \
    return String::New("")
#define API_RETURN_INT(__int)                                           \
    return Integer::New(__int)

#define API_DEF_FUNC(__name)                                            \
    weechat_obj->Set(String::New(#__name),                              \
                     FunctionTemplate::New(weechat_js_api_##__name));
#define API_FUNC_DEF(__name)                                            \
    static Handle<Value> weechat_js_api_##__name (const Arguments &args)

API_FUNC_DEF(register)
{
    API_FUNC(0, "register", API_RETURN_ERROR);
    if (args.Length() != 7)
        API_WRONG_ARGS(API_RETURN_ERROR);
    if (js_registered_script)
    {
        /* script already registered */
        weechat_printf (NULL,
                        weechat_gettext ("%s%s: script \"%s\" already "
                                         "registered (register ignored)"),
                        weechat_prefix ("error"), JS_PLUGIN_NAME,
                        js_registered_script->name);
        API_RETURN_ERROR;
    }

    js_current_script = NULL;
    js_registered_script = NULL;
    String::AsciiValue name(args[0]);
    String::AsciiValue author(args[1]);
    String::AsciiValue version(args[2]);
    String::AsciiValue license(args[3]);
    String::AsciiValue description(args[4]);
    String::AsciiValue shutdown_func(args[5]);
    String::AsciiValue charset(args[6]);

    if (plugin_script_search(weechat_js_plugin, js_scripts, *name))
    {
        /* another script already exists with same name */
        weechat_printf (NULL,
                        weechat_gettext ("%s%s: unable to register script "
                                         "\"%s\" (another script already "
                                         "exists with this name)"),
                        weechat_prefix ("error"), JS_PLUGIN_NAME, *name);
        API_RETURN_ERROR;
    }
    /* register script */
    js_current_script = plugin_script_add (weechat_js_plugin,
                                           &js_scripts, &last_js_script,
                                           (js_current_script_filename) ?
                                           js_current_script_filename : "",
                                           *name, *author, *version,
                                           *license, *description,
                                           *shutdown_func, *charset);

    if (js_current_script)
    {
        js_registered_script = js_current_script;
        if ((weechat_js_plugin->debug >= 2) || !js_quiet)
        {
            weechat_printf (NULL,
                            weechat_gettext ("%s: registered script \"%s\", "
                                             "version %s (%s)"),
                            JS_PLUGIN_NAME, *name, *version, *description);
        }
    }
    else
    {
        API_RETURN_ERROR;
    }

    API_RETURN_OK;
}

API_FUNC_DEF(plugin_get_name)
{
    const char *result;

    API_FUNC(1, "plugin_get_name", API_RETURN_EMPTY);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue plugin(args[0]);
    result = weechat_plugin_get_name(
                (struct t_weechat_plugin *) (API_STR2PTR(*plugin)));

    API_RETURN_STRING(result);
}

API_FUNC_DEF(charset_set)
{
    API_FUNC(1, "charset_set", API_RETURN_ERROR);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_ERROR);

    String::AsciiValue charset(args[0]);
    plugin_script_api_charset_set(js_current_script,
                                  *charset);

    API_RETURN_OK;
}

API_FUNC_DEF(iconv_to_internal)
{
    char *result;

    API_FUNC(1, "iconv_to_internal", API_RETURN_EMPTY);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue charset(args[0]);
    String::AsciiValue string(args[1]);

    result = weechat_iconv_to_internal(*charset, *string);

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(gettext)
{
    const char *result;

    API_FUNC(1, "gettext", API_RETURN_EMPTY);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue string(args[0]);

    result = weechat_gettext(*string);
    API_RETURN_STRING(result);
}

API_FUNC_DEF(ngettext)
{
    const char *result;

    API_FUNC(1, "ngettext", API_RETURN_EMPTY);
    if (args.Length() != 3)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue single(args[0]);
    String::AsciiValue plural(args[1]);
    int count = args[2]->IntegerValue();

    result = weechat_ngettext(*single, *plural, count);
    API_RETURN_STRING(result);
}

API_FUNC_DEF(string_match)
{
    int case_sensitive, value;

    API_FUNC(1, "string_match", API_RETURN_INT(0));
    if (args.Length() != 3)
        API_WRONG_ARGS(API_RETURN_INT(0));

    String::AsciiValue string(args[0]);
    String::AsciiValue mask(args[1]);
    case_sensitive = args[2]->IsFalse() ? 0 : 1;

    value = weechat_string_match(*string, *mask, case_sensitive);

    API_RETURN_INT(value);
}

API_FUNC_DEF(string_has_highlight)
{
    int value;

    API_FUNC(1, "string_has_highlight", API_RETURN_INT(0));
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_INT(0));

    String::AsciiValue string(args[0]);
    String::AsciiValue highlight_words(args[1]);

    value = weechat_string_has_highlight(*string, *highlight_words);

    API_RETURN_INT(value);
}

API_FUNC_DEF(string_has_highlight_regex)
{
    int value;

    API_FUNC(1, "string_has_highlight_regex", API_RETURN_INT(0));
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_INT(0));

    String::AsciiValue string(args[0]);
    String::AsciiValue regex(args[1]);

    value = weechat_string_has_highlight_regex(*string, *regex);

    API_RETURN_INT(value);
}

API_FUNC_DEF(string_mask_to_regex)
{
    const char *result;

    API_FUNC(1, "string_mask_to_regex", API_RETURN_EMPTY);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue mask(args[0]);

    result = weechat_string_mask_to_regex(*mask);

    API_RETURN_STRING(result);
}

API_FUNC_DEF(string_remove_color)
{
    char *result;

    API_FUNC(1, "string_remove_color", API_RETURN_EMPTY);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue string(args[0]);
    String::AsciiValue replacement(args[1]);

    result = weechat_string_remove_color(*string, *replacement);

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(string_is_command_char)
{
    int value;

    API_FUNC(1, "string_is_command_char", API_RETURN_INT(0));
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_INT(0));

    String::AsciiValue string(args[0]);

    value = weechat_string_is_command_char(*string);

    API_RETURN_INT(value);
}

API_FUNC_DEF(string_input_for_buffer)
{
    const char *result;

    API_FUNC(1, "string_input_for_buffer", API_RETURN_EMPTY);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue string(args[0]);

    result = weechat_string_input_for_buffer(*string);

    API_RETURN_STRING(result);
}

API_FUNC_DEF(string_eval_expression)
{
    char *result;
    struct t_hashtable *pointers, *extra_vars;

    API_FUNC(1, "string_eval_expression", API_RETURN_EMPTY);
    if (args.Length() != 3)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue expr(args[0]);
    pointers = weechat_js_object_to_hashtable(args[1]->ToObject(),
                                              WEECHAT_SCRIPT_HASHTABLE_DEFAULT_SIZE,
                                              WEECHAT_HASHTABLE_STRING,
                                              WEECHAT_HASHTABLE_POINTER);
    extra_vars = weechat_js_object_to_hashtable(args[2]->ToObject(),
                                                WEECHAT_SCRIPT_HASHTABLE_DEFAULT_SIZE,
                                                WEECHAT_HASHTABLE_STRING,
                                                WEECHAT_HASHTABLE_STRING);

    result = weechat_string_eval_expression(*expr, pointers, extra_vars);

    if (pointers)
        weechat_hashtable_free(pointers);
    if (extra_vars)
        weechat_hashtable_free(extra_vars);

    API_RETURN_STRING(result);
}

API_FUNC_DEF(mkdir_home)
{
    int mode;

    API_FUNC(1, "mkdir_home", API_RETURN_ERROR);
    if (args.Length() != 2)
        API_RETURN_ERROR;

    String::AsciiValue directory(args[0]);
    mode = args[1]->IntegerValue();

    if (weechat_mkdir_home(*directory, mode))
        API_RETURN_OK;

    API_RETURN_ERROR;
}

API_FUNC_DEF(mkdir)
{
    int mode;

    API_FUNC(1, "mkdir", API_RETURN_ERROR);
    if (args.Length() != 2)
        API_RETURN_ERROR;

    String::AsciiValue directory(args[0]);
    mode = args[1]->IntegerValue();

    if (weechat_mkdir(*directory, mode))
        API_RETURN_OK;

    API_RETURN_ERROR;
}

API_FUNC_DEF(mkdir_parents)
{
    int mode;

    API_FUNC(1, "mkdir_parents", API_RETURN_ERROR);
    if (args.Length() != 2)
        API_RETURN_ERROR;

    String::AsciiValue directory(args[0]);
    mode = args[1]->IntegerValue();

    if (weechat_mkdir_parents(*directory, mode))
        API_RETURN_OK;

    API_RETURN_ERROR;
}

API_FUNC_DEF(list_new)
{
    char *result;

    API_FUNC(1, "list_new", API_RETURN_EMPTY);
    if (args.Length() != 0)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    result = API_PTR2STR(weechat_list_new());

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(list_add)
{
    char *result;

    API_FUNC(1, "list_add", API_RETURN_EMPTY);
    if (args.Length() != 4)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue weelist(args[0]);
    String::AsciiValue data(args[1]);
    String::AsciiValue where(args[2]);
    String::AsciiValue user_data(args[3]);

    result = API_PTR2STR(weechat_list_add((t_weelist *) API_STR2PTR(*weelist),
                                          *data,
                                          *where,
                                          API_STR2PTR(*user_data)));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(list_search)
{
    char *result;

    API_FUNC(1, "list_search", API_RETURN_EMPTY);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue weelist(args[0]);
    String::AsciiValue data(args[1]);

    result = API_PTR2STR(weechat_list_search((t_weelist *) API_STR2PTR(*weelist),
                                             *data));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(list_search_pos)
{
    int pos;

    API_FUNC(1, "list_search_pos", API_RETURN_INT(-1));
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_INT(-1));

    String::AsciiValue weelist(args[0]);
    String::AsciiValue data(args[1]);

    pos = weechat_list_search_pos((t_weelist *) API_STR2PTR(*weelist), *data);

    API_RETURN_INT(pos);
}

API_FUNC_DEF(list_casesearch)
{
    char *result;

    API_FUNC(1, "list_casesearch", API_RETURN_EMPTY);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue weelist(args[0]);
    String::AsciiValue data(args[1]);

    result = API_PTR2STR(weechat_list_casesearch((t_weelist *) API_STR2PTR(*weelist), *data));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(list_casesearch_pos)
{
    int pos;

    API_FUNC(1, "list_casesearch_pos", API_RETURN_INT(-1));
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_INT(-1));

    String::AsciiValue weelist(args[0]);
    String::AsciiValue data(args[1]);

    pos = weechat_list_casesearch_pos((t_weelist *) API_STR2PTR(*weelist), *data);

    API_RETURN_INT(pos);
}

API_FUNC_DEF(list_get)
{
    char *result;
    int position;

    API_FUNC(1, "list_get", API_RETURN_EMPTY);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue weelist(args[0]);
    position = args[1]->IntegerValue();

    result = API_PTR2STR(weechat_list_get((t_weelist *) API_STR2PTR(*weelist), position));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(list_set)
{
    API_FUNC(1, "list_set", API_RETURN_ERROR);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_ERROR);

    String::AsciiValue item(args[0]);
    String::AsciiValue value(args[1]);

    weechat_list_set((t_weelist_item *) API_STR2PTR(*item), *value);

    API_RETURN_OK;
}

API_FUNC_DEF(list_next)
{
    char *result;

    API_FUNC(1, "list_next", API_RETURN_EMPTY);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue item(args[0]);

    result = API_PTR2STR(weechat_list_next((t_weelist_item *) API_STR2PTR(*item)));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(list_prev)
{
    char *result;

    API_FUNC(1, "list_prev", API_RETURN_EMPTY);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue item(args[0]);

    result = API_PTR2STR(weechat_list_prev((t_weelist_item *) API_STR2PTR(*item)));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(list_string)
{
    const char *result;

    API_FUNC(1, "list_string", API_RETURN_EMPTY);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue item(args[0]);

    result = weechat_list_string((t_weelist_item *) API_STR2PTR(*item));

    API_RETURN_STRING(result);
}

API_FUNC_DEF(list_size)
{
    int size;

    API_FUNC(1, "list_size", API_RETURN_INT(0));
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_INT(0));

    String::AsciiValue weelist(args[0]);

    size = weechat_list_size((t_weelist *) API_STR2PTR(*weelist));

    API_RETURN_INT(size);
}

API_FUNC_DEF(list_remove)
{
    API_FUNC(1, "list_remove", API_RETURN_ERROR);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_ERROR);

    String::AsciiValue weelist(args[0]);
    String::AsciiValue item(args[1]);

    weechat_list_remove((t_weelist *) API_STR2PTR(*weelist), (t_weelist_item *) API_STR2PTR(*item));

    API_RETURN_OK;
}

API_FUNC_DEF(list_remove_all)
{
    API_FUNC(1, "list_remove_all", API_RETURN_ERROR);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_ERROR);

    String::AsciiValue weelist(args[0]);

    weechat_list_remove_all((t_weelist *) API_STR2PTR(*weelist));

    API_RETURN_OK;
}

API_FUNC_DEF(list_free)
{
    API_FUNC(1, "list_free", API_RETURN_ERROR);
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_ERROR);

    String::AsciiValue weelist(args[0]);

    weechat_list_free((t_weelist *) API_STR2PTR(*weelist));

    API_RETURN_OK;
}

int weechat_js_api_config_reload_cb(void *data, struct t_config_file *config_file)
{
    return WEECHAT_CONFIG_READ_FILE_NOT_FOUND;
}

API_FUNC_DEF(config_new)
{
    char *result;

    API_FUNC(1, "config_new", API_RETURN_EMPTY);
    if (args.Length() != 3)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue name(args[0]);
    String::AsciiValue function(args[1]);
    String::AsciiValue data(args[2]);

    result = API_PTR2STR(plugin_script_api_config_new(weechat_js_plugin, js_current_script, *name, NULL, *function, *data));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(config_new_section)
{
    int user_can_add_options, user_can_delete_options;
    char *result;

    API_FUNC(1, "config_new_section", API_RETURN_EMPTY);
    if (args.Length() != 14)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue config_file(args[0]);
    String::AsciiValue name(args[1]);
    user_can_add_options = args[2]->IntegerValue();
    user_can_delete_options = args[3]->IntegerValue();
    String::AsciiValue function_read(args[4]);
    String::AsciiValue data_read(args[5]);
    String::AsciiValue function_write(args[6]);
    String::AsciiValue data_write(args[7]);
    String::AsciiValue function_write_default(args[8]);
    String::AsciiValue data_write_default(args[9]);
    String::AsciiValue function_create_option(args[10]);
    String::AsciiValue data_create_option(args[11]);
    String::AsciiValue function_delete_option(args[12]);
    String::AsciiValue data_delete_option(args[13]);

    result = API_PTR2STR(plugin_script_api_config_new_section (weechat_js_plugin,
                                                               js_current_script,
                                                               (t_config_file *) API_STR2PTR(*config_file),
                                                               *name,
                                                               user_can_add_options,
                                                               user_can_delete_options,
                                                               NULL,
                                                               *function_read,
                                                               *data_read,
                                                               NULL,
                                                               *function_write,
                                                               *data_write,
                                                               NULL,
                                                               *function_write_default,
                                                               *data_write_default,
                                                               NULL,
                                                               *function_create_option,
                                                               *data_create_option,
                                                               NULL,
                                                               *function_delete_option,
                                                               *data_delete_option));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(config_search_section)
{
    char *result;

    API_FUNC(1, "config_search_section", API_RETURN_EMPTY);
    if (args.Length() != 2)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue config_file(args[0]);
    String::AsciiValue section_name(args[1]);

    result = API_PTR2STR(weechat_config_search_section((t_config_file *) API_STR2PTR(*config_file), *section_name));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(config_new_option)
{
    char *result;
    int min, max, null_value_allowed;

    API_FUNC(1, "config_new_option", API_RETURN_EMPTY);
    if (args.Length() != 17)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue config_file(args[0]);
    String::AsciiValue section(args[1]);
    String::AsciiValue name(args[2]);
    String::AsciiValue type(args[3]);
    String::AsciiValue description(args[4]);
    String::AsciiValue string_values(args[5]);
    min = args[6]->IntegerValue();
    max = args[7]->IntegerValue();
    String::AsciiValue default_value(args[8]);
    String::AsciiValue value(args[9]);
    null_value_allowed = args[10]->IntegerValue();
    String::AsciiValue function_check_value(args[11]);
    String::AsciiValue data_check_value(args[12]);
    String::AsciiValue function_change(args[13]);
    String::AsciiValue data_change(args[14]);
    String::AsciiValue function_delete(args[15]);
    String::AsciiValue data_delete(args[16]);

    result = API_PTR2STR(plugin_script_api_config_new_option (weechat_js_plugin,
                                                              js_current_script,
                                                              (t_config_file *) API_STR2PTR(*config_file),
                                                              (t_config_section *) API_STR2PTR(*section),
                                                              *name,
                                                              *type,
                                                              *description,
                                                              *string_values,
                                                              min,
                                                              max,
                                                              *default_value,
                                                              *value,
                                                              null_value_allowed,
                                                              NULL,
                                                              *function_check_value,
                                                              *data_check_value,
                                                              NULL,
                                                              *function_change,
                                                              *data_change,
                                                              NULL,
                                                              *function_delete,
                                                              *data_delete));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(config_search_option)
{
    char *result;

    API_FUNC(1, "config_search_option", API_RETURN_EMPTY);
    if (args.Length() != 3)
        API_WRONG_ARGS(API_RETURN_EMPTY);

    String::AsciiValue config_file(args[0]);
    String::AsciiValue section(args[1]);
    String::AsciiValue option_name(args[2]);

    result = API_PTR2STR(weechat_config_search_option((t_config_file *) API_STR2PTR(*config_file), (t_config_section *) API_STR2PTR(*section), *option_name));

    API_RETURN_STRING_FREE(result);
}

API_FUNC_DEF(config_string_to_boolean)
{
    int value;

    API_FUNC(1, "config_string_to_boolean", API_RETURN_INT(0));
    if (args.Length() != 1)
        API_WRONG_ARGS(API_RETURN_INT(0));

    String::AsciiValue text(args[0]);

    value = weechat_config_string_to_boolean(*text);

    API_RETURN_INT(value);
}

void
WeechatJsCore::loadLibs()
{
    Local<ObjectTemplate> weechat_obj = ObjectTemplate::New();

    API_DEF_FUNC(register);
    API_DEF_FUNC(plugin_get_name);
    API_DEF_FUNC(charset_set);
    API_DEF_FUNC(iconv_to_internal);
    API_DEF_FUNC(gettext);
    API_DEF_FUNC(ngettext);
    API_DEF_FUNC(string_match);
    API_DEF_FUNC(string_has_highlight);
    API_DEF_FUNC(string_has_highlight_regex);
    API_DEF_FUNC(string_mask_to_regex);
    API_DEF_FUNC(string_remove_color);
    API_DEF_FUNC(string_is_command_char);
    API_DEF_FUNC(string_input_for_buffer);
    API_DEF_FUNC(string_eval_expression);
    API_DEF_FUNC(mkdir_home);
    API_DEF_FUNC(mkdir);
    API_DEF_FUNC(mkdir_parents);
    API_DEF_FUNC(list_new);
    API_DEF_FUNC(list_add);
    API_DEF_FUNC(list_search);
    API_DEF_FUNC(list_search_pos);
    API_DEF_FUNC(list_casesearch);
    API_DEF_FUNC(list_casesearch_pos);
    API_DEF_FUNC(list_get);
    API_DEF_FUNC(list_set)
    API_DEF_FUNC(list_next);
    API_DEF_FUNC(list_prev);
    API_DEF_FUNC(list_string);
    API_DEF_FUNC(list_size);
    API_DEF_FUNC(list_remove);
    API_DEF_FUNC(list_remove_all);
    API_DEF_FUNC(list_free);
    API_DEF_FUNC(config_new);
    API_DEF_FUNC(config_new_section);
    API_DEF_FUNC(config_search_section);
    API_DEF_FUNC(config_new_option);
    API_DEF_FUNC(config_search_option);
    API_DEF_FUNC(config_string_to_boolean);

    this->addGlobal("weechat", weechat_obj);
}

static void
weechat_js_hashtable_map_cb(void *data,
                            struct t_hashtable *hashtable,
                            const char *key,
                            const char *value)
{
    Handle<Object> *obj = (Handle<Object> *) data;

    (*obj)->Set(String::New(key), String::New(value));
}

Handle<Object> weechat_js_hashtable_to_object(struct t_hashtable *hashtable)
{
    Handle<Object> obj = Object::New();

    weechat_hashtable_map_string(hashtable,
                                 &weechat_js_hashtable_map_cb,
                                 &obj);
    return obj;
}

struct t_hashtable *weechat_js_object_to_hashtable(Handle<Object> obj,
                                                   int size,
                                                   const char *type_keys,
                                                   const char *type_values)
{
    struct t_hashtable *hashtable;
    unsigned int i;
    Handle<Array> keys;
    Handle<Value> key, value;

    hashtable = weechat_hashtable_new(size, type_keys, type_values,
                                      NULL, NULL);

    if (hashtable)
    {
        keys = obj->GetOwnPropertyNames();
        for (i = 0; i < keys->Length(); i++)
        {
            key = keys->Get(i);
            value = obj->Get(key);
            String::AsciiValue key_str(key);
            String::AsciiValue value_str(value);
            if (strcmp(type_values, WEECHAT_HASHTABLE_STRING) == 0)
                weechat_hashtable_set(hashtable, *key_str, *value_str);
            else
                weechat_hashtable_set(hashtable, *key_str,
                                      plugin_script_str2ptr(weechat_js_plugin,
                                                            NULL, NULL,
                                                            *value_str));
        }
    }
    return hashtable;
}
