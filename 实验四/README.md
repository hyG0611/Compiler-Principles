# 实验四 · LLVM IR 代码生成

本实验沿用实验三的 Flex/Bison 前端，通过遍历 `past` AST 生成 LLVM IR。生成器支持整数和浮点数、全局和局部变量、数组及数组形参、同文件函数调用、赋值、短路逻辑、`if/else`、`while`、`break`、`continue` 和 `return`。

## 一、用 Clang 观察 IR

将 Clang 加入 PATH 后，在实验四目录执行以下命令。

```powershell
clang -emit-llvm -S ./test.c -o test.ll
```

`test.ll` 是 Clang 21.1.6 的原始输出。用同目录的 LLVM 6 HTML 手册查阅 `alloca`、`store`、`load`、`srem`、`icmp`、`br`、`phi` 和 `ret`。生成器与讲义一样使用带类型的指针，例如 `i32*`。Clang 21 生成的 IR 使用 `ptr`。

`test.c` 中的 `a = 10` 对应把 10 写到 `alloca` 分配的栈槽。计算 `a % 3` 时先 `load` 得到 `a` 的当前值，再执行有符号取余 `srem`，最后用 `ret` 返回结果 1。生成器输出在 `generated-test.ll`，每行一条指令，带基本块标签和函数边界。

## 二、从 AST 生成 IR

解析器把运算符记在 `ivalue`，把左右子树记在 `left` 和 `right`，`if` 的条件记在 `if_cond`。生成器先为所有局部变量在入口块分配栈槽，再按语句执行顺序生成初始化和赋值。块结束时恢复外层变量绑定，允许内层声明遮蔽同名变量。

| 函数 | 生成方式 |
| --- | --- |
| `genArithmeticExpr` | 递归生成操作数，整数运算映射为 `add/sub/mul/sdiv/srem`，浮点运算使用对应的 `fadd/fsub/fmul/fdiv`。 |
| `genLogicExpr` | 整数比较用 `icmp`，浮点比较用 `fcmp`。短路逻辑用 `br` 决定是否计算右操作数，需要产生逻辑值时用 `phi` 合并结果。 |
| `genAssignStmt` | 求出右侧值和左侧地址，再用 `store` 写入。 |
| `genIfStmt` | 条件为真跳到 then，为假跳到 else 或汇合块。已执行 `ret`、`break` 或 `continue` 的块已经有终结指令。 |
| `genWhileStmt` | 先跳到条件块，条件为真进入循环体，为假退出。正常执行完循环体和 `continue` 都回到条件块，`break` 跳到循环出口。 |

五个函数使用题目指定的 `(past node, char *result)`。表达式函数将指令追加到 `result`，并在生成器内部保留结果操作数供父结点使用。默认由 `genSampleProgram` 输出适合 iCoding 文本评测的指令。使用 `--llvm` 时，由 `genProgram` 生成可交给 Clang 编译的 LLVM IR。

## 三、构建与验证

确保 `bison`、`flex`、MinGW GCC 和 Clang 可从 PATH 调用。在实验四目录打开 PowerShell，从源码构建后再运行生成器。

```powershell
.\build.ps1
.\codegen.exe .\test.c
cmd /c "codegen.exe --llvm < test.c > generated-test.ll"
cmd /c "codegen.exe --llvm < control.c > generated-control.ll"

clang -Wno-override-module -c generated-control.ll -o control.o
gcc control.o -o control.exe
.\control.exe
$LASTEXITCODE
```

`test.c` 和其生成 IR 的退出码均为 1。`control.c` 和其生成 IR 的退出码均为 10。后者覆盖循环、嵌套条件、两种短路逻辑、局部变量遮蔽以及 `break` 和 `continue`。其中的 `1 / zero` 位于按短路规则跳过的分支。

最终通过 iCoding 的 25 项测试。本地用 Clang 编译运行生成的 IR，另用 25 组同课程公开样例比对指令文本。[公开样例来源](https://github.com/MilLoong/2025-UESTC-Compiler-Experiment/tree/master/lab4/lab4-example)

## 四、编译错误排查

- `fatal error: lrparser.tab.h: No such file or directory`：直接用 GCC 编译 C 文件时，Bison 尚未生成头文件。先在实验四目录运行 `.\build.ps1`。
- `multiple definition of main`：`gcc *.c` 会把编译器入口 `main.c` 和输入程序 `test.c`、`control.c` 一起链接。构建脚本只编译编译器源文件，两个输入程序应通过标准输入交给 `codegen.exe`。IDE 的“编译当前文件”也应改为运行构建脚本。

也可以使用 `mingw32-make codegen.exe` 构建。在 Linux 下可运行 `make`，生成 `genllvm`。
