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
#ifndef __WEECHAT_JS_CORE_H_
#define __WEECHAT_JS_CORE_H_

#include <cstdio>
#include <v8.h>

class WeechatJsCore
{
public:
    WeechatJsCore(void);
    ~WeechatJsCore(void);

    bool load(v8::Handle<v8::String>);
    bool load(const char *);

    bool loadFile(FILE *);
    bool loadFile(const char *);

    bool execute(void);

    void addGlobal(v8::Handle<v8::String>, v8::Handle<v8::Template>);
    void addGlobal(const char *, v8::Handle<v8::Template>);

    void loadLibs(void);

private:
    v8::HandleScope handle_scope;
    v8::Handle<v8::ObjectTemplate> global;

    v8::Handle<v8::String> source;
};

extern WeechatJsCore *js_current_core;

#endif /* __WEECHAT_JS_CORE_H_ */
