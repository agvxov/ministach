# Ministach
> Lightweight, heap-allocation-free, thread-safe, bloat-free, header-only C/C++ template string library.

### Usage
Ministach operates on minimal mustach-like templates.

```C
const char * mytemplate = "This the syntax of {{ libname }} looks.";
```

Do note that whitespace inside the "stach" -as I call it-
are ignored
To do something useful with it, we have to compile it first:

```C
ministach_t m;
ministach(&m, mytemplate, my_data_function, NULL);
```

Where `my_data_function()` is a user defined function
that will perform value look-ups according to a key.
The most basic implementation would look something like this:

```C
const char * my_data_function(const char * key, int key_len, void * userdata) {
    if (!strncmp("libname")) { return "ministach"; }
    return NULL; // signal an error on unknown key
}
```

After compiling,
you got yourself an initialized `ministach_t` object.
The fields relevant to you are:

```C
int size;   // number of characters the RENDERED template will take up; NULL NOT ACOUNTED FOR
int status; // non-0 signals an error during template compialation
```

When compiled, you can render the ministach to a new buffer:

```C
char * buf = (char*)malloc(m.size + 1);
ministach_render(m, buf);
puts(buf);
```

For a compilable, simple example, consult `test.c`.

For a more complete example, consult `example.cpp`,
which uses C++ maps.

### More documentation
Assuming the Usage section wasnt enough for you,
consult the header itself,
which I tried to comment.

### Notes
+ the maximum number of staches (template keys inside the template) is hardcoded to 128,
you can find it right at the top of the header
