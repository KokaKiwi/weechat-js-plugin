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
