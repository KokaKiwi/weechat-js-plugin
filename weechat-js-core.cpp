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
#include <cstdio>

extern "C"
{
#include "weechat-plugin.h"
#include "plugin-script.h"
#include "weechat-js.h"
}

#include "weechat-js-core.h"

using namespace v8;

WeechatJsCore *js_current_core;

WeechatJsCore::WeechatJsCore ()
{
    this->global = ObjectTemplate::New();
}

WeechatJsCore::~WeechatJsCore ()
{
    
}

bool
WeechatJsCore::load (Handle<String> source)
{
    this->source = source;

    return true;
}

bool
WeechatJsCore::load (const char *source)
{
    Handle<String> src = String::New(source);

    return this->load(src);
}

bool
WeechatJsCore::loadFile (FILE *fp)
{
    int size, i, read;
    char *source;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    source = new char[size + 1];
    source[size] = 0;
    for (i = 0; i < size;)
    {
        read = static_cast<int>(fread(&source[i], 1, size - i, fp));
        i += read;
    }

    if (!this->load(source))
    {
        delete[] source;
        return false;
    }

    delete[] source;
    return true;
}

bool
WeechatJsCore::loadFile (const char *filename)
{
    FILE *fp;

    fp = fopen(filename, "r");
    if (fp == NULL)
        return false;
    if (!this->loadFile(fp))
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    return true;
}

bool
WeechatJsCore::execute ()
{
    Persistent<Context> context = Context::New(NULL, this->global);
    Context::Scope context_scope(context);
    Handle<Script> script = Script::Compile(this->source);

    script->Run();

    context.Dispose();
    return true;
}

void
WeechatJsCore::addGlobal(Handle<String> key, Handle<Template> val)
{
    this->global->Set(key, val);
}

void
WeechatJsCore::addGlobal(const char *key, Handle<Template> val)
{
    this->addGlobal(String::New(key), val);
}
