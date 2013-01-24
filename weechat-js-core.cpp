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
