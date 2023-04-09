//
// Created by MorbidArk on 2023/3/17.
//

#include <math.h>
#include <string.h>
#include "obj_fn.h"
#include "utils.h"
#include "class.h"
#include "core.h"

#include "Tui.h"

#include "Tuibox.h"

ui_t u;

void draw(ui_box_t *b, char *out)
{
    int x, y, len = 0, max = MAXCACHESIZE;
    
    sprintf(out, "");
    for (y = 0; y < b->h; y++)
    {
        for (x = 0; x < b->w; x++)
        {
            /* Truecolor string to generate gradient */
            len += sprintf(out + len,
                "\x1b[48;2;%i;%i;%im ",
                (int)round(255 * ((double)x / (double)b->w)),
                (int)round(255 * ((double)y / (double)b->h)),
                (int)round(255 * ((double)x * (double)y / ((double)b->w * (double)b->h))));
            
            if (len + 1024 > max)
            {
                out = realloc(out, (max *= 2));
            }
        }
        strcat(out + len, "\x1b[0m\n");
    }
    printf("%s", out);
}

/* Function that runs on box click */
void click(ui_box_t *b, int x, int y)
{
    b->data1 = "\x1b[0m                \n  you clicked me!  \n                ",
        ui_draw_one(b, 1, &u);
}

/* Function that runs on box hover */
void hover(ui_box_t *b, int x, int y, int down)
{
    b->data1 = "\x1b[0m                \n  you hovered me!  \n                ",
        ui_draw_one(b, 1, &u);
}

void stop()
{
    ui_free(&u);
    exit(0);
}

bool primTuiDemo(VM *vm, Value *args){

    ui_new(0, &u);

    ui_add(
        1, 1,
        u.ws.ws_col, u.ws.ws_row,
        0,
        NULL, 0,
        draw,
        NULL,
        NULL,
        NULL,
        NULL,
        &u
    );
    
    ui_text(
        ui_center_x(19, &u), UI_CENTER_Y,
        "\x1b[0m                   \n    click on me!   \n                   ",
        0,
        click, hover,
        &u
    );
    
    /* Register an event on the q key */
    ui_key("q", stop, &u);
    
    /* Render the screen */
    ui_draw(&u);

    ui_loop(&u)
    {
        /* Check for mouse/keyboard events */
        ui_update(&u);
    }

    RET_NUM(123)
}

/* 测试调用脚本函数 */
bool _primTestParse(VM *vm, Value *args){

    ObjFn *objFn = VALUE_TO_OBJFN(args[1]);
    
    extern Value moduleName;
    executeModule(vm, moduleName, "a.call()");

    RET_NULL
}

void extenTuiBind(VM *vm, ObjModule *coreModule){
    Class *testClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Tui"));
    PRIM_METHOD_BIND(testClass->objHeader.class, "testFun_()", primTuiDemo);
    PRIM_METHOD_BIND(testClass->objHeader.class, "test_(_)", _primTestParse);
}