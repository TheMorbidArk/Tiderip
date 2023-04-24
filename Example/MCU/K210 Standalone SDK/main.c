/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cli.h"
#include "common.h"
#include "core.h"
#include "printf.h"
#include "syscalls.h"
#include <bsp.h>
#include <sysctl.h>

int main(void)
{

    printf("Tiderip Version:%s\r\n", VERSION);
    VM *vm = newVM();
       
    char *line = "System.println(\"Hello World !!!\")";

    executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "cli", 3)), line);

    while(1){

    }

    return 0;

}

void runLed(void){

    printf("Tiderip Version:%s\r\n", VERSION);
    VM *vm = newVM();
       
    char *line = "Led.ledInit()\n"
                 "Led.ledSet(0,0)\n"
                 "Led.ledSet(1,0)";

    executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "cli", 3)), line);

    while(1){

    }

    return 0;
}

void runCli(void){

    printf("Tiderip Version:%s\r\n", VERSION);
    VM *vm = newVM();
       
    char *line = (char*)malloc(sizeof(char) * 255);

    // 命令动态监控
    while (line != NULL)
    {
        puts(">>> ");
        sys_stdin_flush();
        gets(line);
        if (!strncmp(line, "exit", 4))
        {
            break;
        }
        
        executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "cli", 3)), line);
        free(line);
    }

    while(1){
        
    }

    return ;

}

void runText(void){
    printf("Tiderip Version:%s\r\n", VERSION);
    VM *vm = newVM();
       
    char *line ="System.println(\"Hello World\")"
                "\n"
                "Tide a = 5\n"
                "Tide b = 5.23\n"
                "Tide c = \"abcd\""
                "Tide d = [1,2,3,4]\n"
                "Tide e = {\n"
                "        \"a\":1,"
                "        \"b\":2,"
                "        \"c\":3"
                "        }\n"
                "\n"
                "System.println(\"A:%(a) B:%(b) C:%(c) \r\nD:%(d) \r\nE:%(e)\")"
                "\n"
                "fun Fun_1(a,b){\n"
                "    return a+b\n"
                "}\n"
                "System.println(\"Fun_1(a,b):%(Fun_1(\"Fun\",\"_Define\"))\")"
                "\n"
                "Tide Fun_2 = Fn.new{|a,b|\n"
                "    return a+b\n"
                "}\n"
                "System.println(\"Fun_2(a,b):%(Fun_2.call(\"Class\",\"_Fn\"))\")"
                "\n"
                "fun Fun_3(){\n"
                "\n"
                "    Tide x = 0\n"
                "    if (x == 0){\n"
                "        System.println(\"-x-\")"
                "    }elif(x == 1){\n"
                "         System.println(\"0x0\")"
                "    }else{\n"
                "        System.println(\"*o*\")"
                "    }\n"
                "\n"
                "    Tide arr = [\"H\",\"e\",\"l\",\"l\",\"o\"]"
                "\n"
                "    System.println(\"for-each:\")"
                "    for i (arr){\n"
                "        System.print(i+\" \")"
                "    }\n"
                "    System.println()\n"
                "\n"
                "    System.println(\"index-range:\")"
                "    for i (0..4){\n"
                "        System.print(arr[i]+\" \")"
                "    }\n"
                "    System.println()\n"
                "\n"
                "    Tide idx = 0\n"
                "    System.println(\"While:\")"
                "    while(idx < 10){\n"
                "        idx=idx+1\n"
                "    }\n"
                "    System.println(idx)\n"
                "\n"
                "}\n"
                "Fun_3()\n";
    

    executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "CodeText", 8)), line);

    return ;
}