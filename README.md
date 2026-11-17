# 编译原理学习资料

UESTC信软学院
个人编译原理课程学习资料，包含章节讲义、SysY 语言规范、实验指导和代码。

## 章节资料

根目录的七份讲义依次涵盖引论、词法分析、语法分析、语义分析、中间代码、运行时刻环境、代码优化与生成。[SysY 2022 语言定义](SysY2022语言定义-V1.pdf) 可用于查阅语言规则。

## 实验

| 目录 | 内容 |
| --- | --- |
| [实验一](实验一/) | 词法分析，包含 Flex 规则和记号定义 |
| [实验二](实验二/) | 递归下降语法分析，包含课程模板中的实现片段、表达式解析示例和 SysY 文法 |
| [实验三](实验三/) | LR 语法分析，包含 Flex/Bison 示例和实验提交材料 |
| [实验四](实验四/) | 实现从 SysY AST 到 LLVM IR 的代码生成，包含构建与运行说明 |
| [实验五](实验五/) | 代码优化，包含优化器源码 |

## 构建与运行

以下命令使用 Windows PowerShell 和 MinGW GCC。构建实验一需要 Flex，构建实验三需要 Flex 和 Bison。

### 实验一

在 `实验一` 目录执行以下命令。程序从标准输入读取源代码并输出词法记号。

```powershell
flex -o lex.yy.c lexer.l
gcc lex.yy.c -o lexer.exe
'int main(){return 42;}' | .\lexer.exe
```

### 实验二

`实验二/.c` 是课程模板中的实现片段，依赖模板提供的 `rdlab2.h` 和相关实现。`rdparser.c` 是讲义中的表达式解析示例，依赖提供 `yylex`、`yylval` 和 `yytext` 的配套词法分析器。仓库现有文件不足以独立构建这两份程序。

### 实验三

在 `实验三` 目录执行以下命令。提交版源码位于 [上传目录](实验三/上传/)，运行后输出抽象语法树。

```powershell
Set-Location .\上传
bison -d -o lrparser.tab.c lrparser.y
flex -o lrlex.yy.c lrlex.l
gcc main.c ast.c lrparser.tab.c lrlex.yy.c -o parser.exe
Get-Content -Raw ..\text.c | .\parser.exe
```

### 实验四

[实验四 README](实验四/README.md) 提供构建步骤和 LLVM IR 验证命令。[提交文件](实验四/提交文件/) 包含 iCoding 所需的九个源文件和头文件。

### 实验五

在 `实验五` 目录执行以下命令。输入首行为指令条数，后续每行一条三地址指令。该用例的优化结果为 `print 5`。

```powershell
gcc -std=c11 -Wall -Wextra experiment5_optimizer.c -o optimizer.exe
@'
4
a = 2
b = a + 3
c = b * 1
print c
'@ | .\optimizer.exe
```


## LLVM 安装包

根目录的 `LLVM-21.1.6-win64.exe` 通过 Git LFS 保存。克隆前可先运行 `git lfs install`，已有本地副本可运行 `git lfs pull` 获取安装包。
