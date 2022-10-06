# VanTide-Scripting-Language

## 介绍

基于C实现的脚本语言编译器

- 虚拟机采用大端存储
- 实现基于栈的虚拟机

## 优化方案
- <import 模块>时新建线程对导入的模块文件进行分析
- < GetIndexFromSymbolTable > 函数使用红黑树优化搜索算法
  - 符号表采用红黑树算法重构
  - 使算法运行速度从 **O(n) => O(log<sup>n</sup>)**