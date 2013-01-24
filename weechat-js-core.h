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
