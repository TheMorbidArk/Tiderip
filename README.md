# Tiderip-Scripting-Language

## 介绍

基于C实现的脚本语言<Tiderip>の解释器

- 虚拟机采用大端存储
- 实现基于栈的虚拟机

> **文件树:**  
> &emsp;Tiderip  
> &emsp;&emsp;├─Cli  
> &emsp;&emsp;├─Compiler  
> &emsp;&emsp;├─include  
> &emsp;&emsp;├─Object  
> &emsp;&emsp;├─Parser  
> &emsp;&emsp;└─VM

## 优化方案

- < GetIndexFromSymbolTable > 函数使用红黑树优化搜索算法
    - 符号表采用红黑树算法重构
    - 使算法运行速度从 **O(n) => O(log<sup>n</sup>)**
    - [ ] 增加 < elif > 语法

## BUG
< elif >可独立存在