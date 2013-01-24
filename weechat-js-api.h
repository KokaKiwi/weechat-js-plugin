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

#ifndef __WEECHAT_JS_API_H_
#define __WEECHAT_JS_API_H_

#include <v8.h>

extern "C"
{
#include "weechat-plugin.h"
}

using namespace v8;

extern Handle<Object> weechat_js_hashtable_to_object(struct t_hashtable *hashtable);
extern struct t_hashtable *weechat_js_object_to_hashtable(Handle<Object> obj,
                                                         int size,
                                                         const char *type_keys,
                                                         const char *type_values);

#endif
