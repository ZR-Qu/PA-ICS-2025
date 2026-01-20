# PA0 2025.11.22 

> PA 什么意思？

​	Programming Assignment 小型项目



>   实验要求

通过C语言开发一款模拟器以及上层的软件(裸机运行时环境, 简易操作系统等)

实现一个经过简化但功能完备的x86/mips32/riscv32(64)模拟器NEMU(NJU EMUlator), 最终在NEMU上运行游戏"仙剑奇侠传"



>   **RTFSC**???

去读他妈源代码(**R**ead **T**he **F**ucking **S**ource **C**ode /**M**anual)



>   提问

```
Q: 我的程序在xxx的情况下段错误了, 我进行了以下尝试
* 首先我做了aaa, 现象是AAA, 我得到的结论是XXX
* 然后我做了bbb, 现象是BBB, 我得到的结论是YYY
* 我还做了ccc, 现象是CCC, 我得到的结论是ZZZ

综上, 我觉得问题可能出在yyy, 但我接下来没有思路了, 我的分析和理解是否正确? 或者我忽略了什么吗?
```



>   STFW

STFW（search the friend web）: 至少需要几分钟，而且有不少失败尝试 



# PA1 2025.11.24 

>   NEMU?

一款经过简化的计算机全系统模拟器

>   ccache

再次编译时可跳过=完全重复的编译过程, 加速编译过程

>   ISC

[指令集架构](https://en.wikipedia.org/wiki/Instruction_set_architecture)

ISA是软件和硬件之间的接口



CPU创造了寄存器, 可以让CPU把正在处理中的数据暂时存放在其中.

执行到哪一条指令 -> 程序计数器（PC / EIP）



>   blt r2, 100, 2

**B**ranch if **L**ess **T**han

如果 r2 的值小于 100，则程序计数器（PC）向前跳转 2 条指令



>   最简单的真实计算机需要满足哪些条件

有存储器, 有PC, 有寄存器, 有加法器



NEMU中模拟的计算机称为"客户(guest)计算机", 在NEMU中运行的程序称为"客户程序".

## 拿到源代码，先做什么？

>   找main函数



 `grep -n main $(find . -name "*.c")`

1.  **`find . -name "\*.c"`** - 查找当前目录及子目录中所有 `.c` 文件
2.  **`$(...)`** - 命令替换，将 find 的结果作为参数传递给 grep
3.  **`grep -n main`** - 在文件中搜索 "main" 并显示行号



 `find . | xargs grep --color -nse '\<main\>'`

1.  **`find .`** - 查找当前目录及子目录中的所有文件和目录
2.  **`| xargs`** - 将 find 的输出作为参数传递给后面的命令
3.  **`grep --color -nse '\<main\>'`**：（`--color` - 高亮显示匹配的文本 `-n` - 显示行号 `-s` - 静默模式，不显示错误信息 `-e '\<main\>'` - 搜索单词 "main"）



>   `grep -n '\<main\>' $(find . -name "*.c")` 更精准 不会有remain这种



> 配置系统kconfig

解决配置之间的依赖问题

修改选项A之后, 可能会忘记修改和选项A有关联的选项B. 配置系统的出现则是为了解决这个问题.



>   getopt_long 

解析命令行参数



>   客户程序

运行在 NEMU（模拟器） 上的 “目标程序”，是模拟器要模拟执行的对象

关键特性：

-   与 ISA（指令集架构）强相关：不同 ISA（如 x86、RISC-V）的指令不同，客户程序的指令必须匹配目标 ISA（就像 “你好” 的不同语言表达）。
-   内置版本极简：NEMU 自带的内置客户程序（存放在 `nemu/src/isa/$ISA/init.c`）仅含少量指令，不做实际业务逻辑，主要用于测试模拟器基础执行流程。



>   如果在运行NEMU之后直接键入`q`退出, 你会发现终端输出了一些错误信息. 请分析这个错误信息是什么原因造成的, 然后尝试在NEMU中修复它.

报错：

(nemu) q

make: *** [/home/qzr/ics2025/nemu/scripts/native.mk:38：run] 错误 1



解决方案：
nemu/src/nemu-main.c中：

```
return is_exit_status_bad();
```

查看is_exit_status_bad();

nemu/src/utils/state.c中

```
int is_exit_status_bad() {
  int good = (nemu_state.state == NEMU_END && nemu_state.halt_ret == 0) || //程序跑完了且结果是0
    (nemu_state.state == NEMU_QUIT); //用户主动要求退出 enum中NEMU_QUIT为4
  return !good; //good = 1, 返回0，表示正常
}
```

nemu/src/monitor/sdb/sdb.c中

```
if (cmd_table[i].handler(args) < 0) { return; } //调用该函数，返回值小于0 返回
```

```
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}
```

cmd_q中缺少了nemu_state.state = NEMU_QUIT

修改：

```
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}
```

提示NEMU_QUIT未定义

引入头文件

```
#include "utils.h"
```

>   nemu_state.state的声明

grep -r "nemu_state" include/

include/cpu/cpu.h:void set_nemu_state(int state, vaddr_t pc, int halt_ret);

nemu/src/monitor/sdb/sdb.c中已经引用了\#include <cpu/cpu.h>



### 修复 NEMU 退出时的报错

>   在运行 NEMU 后直接键入 `q` 退出，终端未正常结束，而是输出了 `make` 构建错误的提示信息。

**报错信息：**
```text
(nemu) q
make: *** [/home/qzr/ics2025/nemu/scripts/native.mk:38：run] 错误 1
```

#### 原因分析

#### 2.1 Make 的报错机制

`make` 报错 "错误 1" 是因为 NEMU 程序的 `main` 函数返回了非 0 值。

#### 2.2 代码追踪

查看 `nemu/src/nemu-main.c`，程序退出的返回值由 `is_exit_status_bad()` 决定：

```c
int is_exit_status_bad() {
  int good = (nemu_state.state == NEMU_END && nemu_state.halt_ret == 0) ||
             (nemu_state.state == NEMU_QUIT); //用户主动要求退出 enum中NEMU_QUIT为4
  return !good;	//good = 1, 返回0，表示正常
}
```

**分析：** 要让函数返回 0（正常退出），`good` 必须为真。这意味着当用户输入 `q` 时，全局变量 `nemu_state.state` 必须被设置为 `NEMU_QUIT`。



#### 2.3 定位代码

查看处理 `q` 命令的函数 `cmd_q`（位于 `nemu/src/monitor/sdb/sdb.c`）：

```
static int cmd_q(char *args) {
  return -1; 
}
```

**结论：** `cmd_q` 函数虽然让主循环退出了，但没有更新状态。导致 `main` 函数检查时，认为程序是“异常终止”的，从而触发报错。



#### 3. 解决方案

需要修改 `nemu/src/monitor/sdb/sdb.c` 文件。

##### 步骤 1：引入必要的头文件

为了使用 `nemu_state` 变量和 `NEMU_QUIT` 枚举值，需要确保引用了正确的头文件。 

>   *(注：根据 Grep 结果，`nemu_state` 声明于 `include/utils.h`)*

```
#include <utils.h> 
```



##### 步骤 2：修改 cmd_q 函数

在函数返回前，显式将状态修改为 `NEMU_QUIT`。

```
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}
```



# PA1.1 2026.1.19

运行NEMU

```
cd /home/qzr/ics2025/nemu
make run
```

sdb 调试器



## 阶段目标

>   实现简易调试器，完成以下几个功能

1.实现 `si` 命令 (单步执行)

2.实现 `info r` 命令 (打印寄存器)

3.实现 `x` 命令 (扫描内存)



###实现si

```
nemu/src/monitor/sdb/sdb.c
```

加一条

![image-20260119213319297](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223814221.png)

cmd_si

传入数字

![image-20260119213909904](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223820541.png)

测试

![image-20260119214012330](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223824579.png)



### 实现info

info r  		打印寄存器状态
info w   	打印监视点信息

cmd_info

传入r/w

![image-20260119230811127](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223827558.png)

isa_reg_display();在nemu/src/isa/$ISA/reg.c

需要用到

`cpu.pc`: 当前指令地址 (PC)。

`cpu.gpr[i]`: 第 `i` 个通用寄存器的值。

`regs[i]`: 第 `i` 个通用寄存器的名字（字符串）

![image-20260119230825615](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223831842.png)

![image-20260119231027747](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223833974.png)

### 实现x

调用 NEMU 的内存读取 API (`vaddr_read`）

```
#include <memory/vaddr.h>
```

![image-20260119232120738](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223836898.png)

![image-20260119232153127](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223840094.png)



#PA1.2 2026.1.19(复现时先去看bug部分)

词法分析：识别出表达式中的单元

单元：token

识别出其中的token -> `make_token()`函数

## 阶段目标

-   ### 1. 词法分析 (Lexical Analysis)

    **目标**：把用户输入的字符串（如 `"5 + 4 * 3"`）切割成一个个有意义的单元（Token）。

    -   **完善正则表达式规则**： 在 `nemu/src/monitor/sdb/expr.c` 中，找到 `rules` 数组。你需要为以下类型添加正则表达式规则：
        -   十进制整数 (例如 `[0-9]+`)
        -   十六进制整数 (例如 `0x[0-9a-fA-F]+`)
        -   运算符：`+`, `-`, `*`, `/`
        -   括号：`(`, `)`
        -   **注意**：空格的规则已经有了，但你需要确保它被忽略（`TK_NOTYPE`）。
    -   **完善 `make_token` 函数**： 当正则表达式匹配成功后，你需要把识别出来的信息存入 `tokens` 数组中：
        -   记录 `type`（类型）。
        -   **关键点**：对于数字（整数、十六进制），你还需要把它的具体文本值（如 `"123"`）复制到 `str` 成员中，否则后面没法算值。
        -   **防坑**：注意检查 `str` 缓冲区是否溢出 (Buffer Overflow)。

    ------

    ### 2. 递归求值 (Recursive Evaluation)

    **目标**：实现核心函数 `eval(p, q)`，计算从 `tokens[p]` 到 `tokens[q]` 的值。

    你需要实现以下 3 个关键逻辑：

    -   **实现 `check_parentheses(p, q)`**：
        -   判断表达式是否被一对匹配的括号包围。例如 `(4+3)` 返回 true，但 `(4+3)*(2-1)` 返回 false。
        -   你需要用栈或者计数器来检查左右括号是否匹配。
    -   **寻找“主运算符” (Main Operator)**：
        -   遍历 `p` 到 `q` 之间的 token。
        -   **规则**：
            1.  必须不在括号内。
            2.  优先级最低（先算乘除，后算加减，所以主运算符通常是加减）。
            3.  如果有多个优先级相同的，找**最右边**那个（因为结合性）。
    -   **实现递归逻辑 `eval`**： 根据文档提供的伪代码框架填空：
        1.  **Base Case**：如果是单个数字，直接用 `strtoul` 或 `sscanf` 转成数字返回。
        2.  **去掉括号**：如果 `check_parentheses` 为真，递归调用 `eval(p+1, q-1)`。
        3.  **一般情况**：找到主运算符 `op`，递归计算 `val1 = eval(p, op-1)` 和 `val2 = eval(op+1, q)`，然后根据 `op` 的类型进行计算（`val1 + val2` 等）。

    ------

    ### 3. 实现表达式生成器 (Infrastructure)

    **目标**：生成大量随机测试用例，确信你的 `expr` 是对的。

    -   **修改 `nemu/tools/gen-expr/gen-expr.c`**：
        -   实现 `gen_rand_expr()` 函数。
        -   利用随机数递归生成合法的表达式字符串（数字、括号、运算符）。
        -   处理细节：插入随机空格、避免除 0、确保使用无符号运算 (`uint32_t`)。
        -   **生成测试文件**：运行它生成 `input` 文件。
    -   **修改 NEMU 的 `main` 函数 (临时)**：
        -   让 NEMU 启动时读取 `input` 文件。
        -   循环调用你写的 `expr()`，并与文件里的标准结果对比。
        -   如果全部通过，说明你的表达式求值没问题。

    ------

    ### 4. 接入简易调试器 (SDB)

    **目标**：让用户真正能用。

    -   **实现 `p` 命令**：
        -   在 `sdb.c` 中注册 `p` 命令。
        -   调用 `expr()` 计算参数的值并打印。
    -   **选做任务 (推荐做)**：
        -   实现负数支持（区分减号 `-` 和负号 `-`）。如果不做，以后遇到解引用 `*($eax + -4)` 时会报错。



### 1-词法分析 

nemu/src/monitor/sdb/expr.c

#### 编写正则表达式规则

![image-20260120003650992](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223845866.png)

十六进制在前：正则会优先尝试匹配完整的 `0x123`，匹配成功！



#### 完善 `make_token` 函数

![image-20260120005744460](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223848878.png)

switch前可以约束一下token数量

![image-20260120010007835](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223852609.png)





### 2-添加p操作

1.cmd_p

![image-20260120111838746](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223855280.png)

2.完善expc（）

![image-20260120103408437](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223858111.png)

### 3-实现核心函数 eval(p, q)

3.完善check_parentheses(p, q) 

![image-20260120012717019](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223900622.png)

4.完善eval

```
static word_t eval(int p, int q, bool *success) {
  if(*success == false) return 0;
  if (p > q) {
    *success = false;
    return 0;
  }
  else if (p == q) {  //Single token，应为一个数
    // 十进制数
    if (tokens[p].type == TK_DEC) {
        return strtoul(tokens[p].str, NULL, 10);
    }
    // 十六进制数
    else if (tokens[p].type == TK_HEX) {
        return strtoul(tokens[p].str, NULL, 16);
    }
    
    *success = false;
    return 0;
  }
  else if (check_parentheses(p, q) == true) {
    /* 
    The expression is surrounded by a matched pair of parentheses.
    If that is the case, just throw away the parentheses.
     */
     return eval(p + 1, q - 1, success);
  }
  else {  //p < q
    // 寻找主运算符 从 p 到 q 扫描，找括号外的、优先级最低的、最右边的运算符
    int op_index = -1;
    int op_type = 0;
    int parens_cnt = 0;

    for(int i = p; i <= q; i++){
      if (tokens[i].type == '(') {
        parens_cnt++;
      } else if (tokens[i].type == ')') {
        parens_cnt--;
      }

      if(parens_cnt == 0 && !(tokens[i].type == TK_DEC || tokens[i].type == TK_HEX || tokens[i].type == ')')){
        int cur_op_type = tokens[i].type; 
        if(op_index == -1){
          op_index = i;
          op_type = cur_op_type;
        }else if(cur_op_type == '+'||cur_op_type == '-'){
          op_index = i;
          op_type = cur_op_type;
        }else if(cur_op_type == '*'||cur_op_type == '/'){
          if(op_type != '+' && op_type != '-'){
            op_index = i;
            op_type = cur_op_type;
          }
        }
      }
    }
    if(op_index == -1) assert(0);

    word_t val1 = eval(p, op_index-1, success);
    word_t val2 = eval(op_index+1, q, success);

    word_t res = 0;
    switch(op_type){
      case '+':
        res = val1 + val2;
        break;
      case '-':
        res = val1 - val2;
        break;
      case '*':
        res = val1 * val2;
        break;
      case '/':
        if(val2){
          res = val1 / val2;
        }else{
          printf("ERR: Division by zero.\n");
          *success = false;
        }
        break;
      default: assert(0);
    }
    return res;
  }
}
```

### 初步测试

![image-20260120112511656](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223907108.png)

![image-20260120112417103](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223909576.png)

### 4-测试工具

```
nemu/tools/gen-expr/gen-expr.c
```

![image-20260120113506625](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223911978.png)

![image-20260120113630345](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223914745.png)

#### 测试

![image-20260120114304941](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223917612.png)

![image-20260120114312778](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223920013.png)

改token缓冲区

/home/qzr/ics2025/nemu/src/monitor/sdb/expr.c

![image-20260120114523244](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223922832.png)

static bool make_token(char *e)

![image-20260120114531444](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223925370.png)

修改main函数

![image-20260120161053074](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223928105.png)

![image-20260120161114995](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223930676.png)

```
#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

#define TEST_EXPR
word_t expr(char *e, bool *success);

void test_expr() {
    printf("Test: Start testing...\n");
    
    FILE *fp = fopen("tools/gen-expr/build/input", "r");
    if (fp == NULL) {
        printf("Error: Cannot open input file.\n");
        return;
    }

    char line[65536];
    int count = 0;
    int failed_count = 0; // 失败的
    int skipped_count = 0; // 跳过的
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        uint32_t expected_res;
        char *expr_str = line;
        
        if (sscanf(line, "%u", &expected_res) != 1) continue; 
        
        while (*expr_str >= '0' && *expr_str <= '9') expr_str++;
        while (*expr_str == ' ') expr_str++; 
        if (*expr_str == 'u' || *expr_str == 'U') expr_str++; // 跳过可能残留的u
        while (*expr_str == ' ') expr_str++;

        bool success;
        word_t actual_res = expr(expr_str, &success);

        if (success == false) {
            // 如果 存在坏例则跳过  【NEMU 报错（比如除0），但 GCC 没报错】
            printf("[Warn] Skipped bad case at line %d (NEMU failed)\n", count);
            skipped_count++;
            count++;
            continue; 
        }

        if (actual_res != expected_res) {
            printf("\n[Error] Test failed!\n");
            printf("Line:     %d\n", count);
            printf("Expr:     %s\n", expr_str);
            printf("Expected: %u\n", expected_res);
            printf("Actual:   %u\n", actual_res);
            failed_count++;
        }
        
        count++;
        if (count % 1000 == 0) {
            printf("Passed %d tests (Skipped %d)...\n", count - failed_count, skipped_count);
        }
    }

    printf("\nTotal: %d, Failed: %d, Skipped: %d\n", count, failed_count, skipped_count);
    if (failed_count == 0) {
        printf("\033[1;32mCongratulation! passed all tests!\033[0m\n");
    } else {
        printf("\033[1;31mSome tests failed. Check output.\033[0m\n");
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

#ifdef TEST_EXPR
  test_expr();
  return 0;
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
```

#### 测试流程

1.生成input

```
~/ics2025/nemu/tools/gen-expr/build$ ./gen-expr 1000 > input
```

2./home/qzr/ics2025/nemu/src/nemu-main.c中

注释掉为关闭测试，取消注释为开启测试

```
// #define TEST_EXPR
```

3.切换到nemu/

```
make run
```

![image-20260120173717393](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223935293.png)



####测试bug-log

##### bug1: word_t

一个超长的表达式

![image-20260120162743832](assets/PA-Log/image-20260120162743832.png)

发现是因为用的 int 要用 uint32_t 即 word_t

在 NEMU 中，`word_t` 通常被定义为 `uint32_t`

```
int val1 = eval(p, op_index-1, success);
int val2 = eval(op_index+1, q, success);
```

修改为

```
word_t val1 = eval(p, op_index-1, success);
word_t val2 = eval(op_index+1, q, success);

word_t res = 0;
```



##### bug2:(gcc按有符号，nemu按无符号)

![image-20260120164622544](assets/PA-Log/image-20260120164622544.png)

```
[Error] Test failed!
Line:     1
Expr:     (((((( (198)+((650*(920)-604 +(438 ) +618)*(371+517*((( ( 258)- (((749)))*898*154)))))) ))))/236+(670)+(402- ((374/(519-(((((129))))))/( 640-640/ ((( 397 /927/980+428 -(209)  )-834)*((322)))/(((948)))* ((267))*((((((646)-207))+423-(967)-((293+353))/290-927))/(419/679+((678))))/(847))-462))) )
Expected: 4291562099
Actual:   14793816
riscv32-nemu-interpreter: src/nemu-main.c:70: test_expr: Assertion `0' failed.
make: *** [/home/qzr/ics2025/nemu/scripts/native.mk:38: run] 已中止 (核心已转储)
```

修改：/home/qzr/ics2025/nemu/tools/gen-expr/gen-expr.c中生成随机数字改为du

![image-20260120214900514](https://pic-obsidian-typora.oss-cn-wuhan-lr.aliyuncs.com/20260120223940013.png)



##### bug3：坏例（nemu任务除0异常而gcc正常）

```
ERR: Division by zero.
ERR: Division by zero.

[Error] expr failed (syntax error?)
Line 96: (省略表达式)
riscv32-nemu-interpreter: src/nemu-main.c:61: test_expr: Assertion `0' failed.
make: *** [/home/qzr/ics2025/nemu/scripts/native.mk:38: run] 已中止 (核心已转储
```

解决方案：修改`test_expr` 跳过坏用例

```
void test_expr() {
    printf("Test: Start testing...\n");
    
    FILE *fp = fopen("tools/gen-expr/build/input", "r");
    if (fp == NULL) {
        printf("Error: Cannot open input file.\n");
        return;
    }

    char line[65536];
    int count = 0;
    int failed_count = 0; // 失败的
    int skipped_count = 0; // 跳过的
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        uint32_t expected_res;
        char *expr_str = line;
        
        if (sscanf(line, "%u", &expected_res) != 1) continue; 
        
        while (*expr_str >= '0' && *expr_str <= '9') expr_str++;
        while (*expr_str == ' ') expr_str++; 
        if (*expr_str == 'u' || *expr_str == 'U') expr_str++; // 跳过可能残留的u
        while (*expr_str == ' ') expr_str++;

        bool success;
        word_t actual_res = expr(expr_str, &success);

        if (success == false) {
            // 如果 存在坏例则跳过  【NEMU 报错（比如除0），但 GCC 没报错】
            printf("[Warn] Skipped bad case at line %d (NEMU failed)\n", count);
            skipped_count++;
            count++;
            continue; 
        }

        if (actual_res != expected_res) {
            printf("\n[Error] Test failed!\n");
            printf("Line:     %d\n", count);
            printf("Expr:     %s\n", expr_str);
            printf("Expected: %u\n", expected_res);
            printf("Actual:   %u\n", actual_res);
            failed_count++;
        }
        
        count++;
        if (count % 1000 == 0) {
            printf("Passed %d tests (Skipped %d)...\n", count - failed_count, skipped_count);
        }
    }

    printf("\nTotal: %d, Failed: %d, Skipped: %d\n", count, failed_count, skipped_count);
    if (failed_count == 0) {
        printf("\033[1;32mCongratulation! passed all tests!\033[0m\n");
    } else {
        printf("\033[1;31mSome tests failed. Check output.\033[0m\n");
    }
    fclose(fp);
}
```

