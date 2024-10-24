// @BAKE gcc $@ -o $*.out -Wall -Wpedantic -ggdb
#include "ministach.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char * my_data_function(const char * key, int key_len, void * userdata) {
    if (!strncmp("owner", key, key_len)) {
        return "my";
    } else
    if (!strncmp("lib", key, key_len)) {
        return "ministach";
    } else
    if (!strncmp("long_msg", key, key_len)) {
        return "long long long long long long message";
    }

    return NULL;
}

void test(const char * template, const char * result) {
    int r;
    static int c = 1;
    printf("# TEST %d -----------------\n", c++);

    ministach_t m;
    ministach_compile(&m, template, my_data_function, NULL);

    if (m.status) { r = m.status; goto end; }

    char * buf = (char*)malloc(m.size+1);
    ministach_render(&m, buf);

    r = strcmp(buf, result);
    puts(buf);

    free(buf);

  end:
    if (r) {
        printf("\nERROR: %d\n", r);
    }
}

signed main(void) {
    {
        const char * template = "This is {{owner}} template string for {{lib}}.";
        const char * result   = "This is my template string for ministach.";
        test(template, result);
    }
    {
        const char * template = "{{lib}}";
        const char * result   = "ministach";
        test(template, result);
    }
    {
        const char * template = "{{ owner }} {{  owner }} {{  owner   }}!";
        const char * result   = "my my my!";
        test(template, result);
    }
    puts("--- Error Batch bellow ---");
    {
        const char * template = "This should {{ error }}!";
        const char * result   = "";
        test(template, result);
    }
    {
        const char * template = "Duplicate error {{ my my }}.";
        const char * result   = "";
        test(template, result);
    }
    {
        const char * template = "Empty error {{   }}.";
        const char * result   = "";
        test(template, result);
    }

    return 0;
}
