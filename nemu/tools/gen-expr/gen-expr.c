/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static int buf_pos = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

// 写入单个字符
static void gen(char c) {
    if (buf_pos < 65500) { // 缓冲区大小约束
        buf[buf_pos++] = c;
    }
}

// 生成随机数字
static void gen_num() {
    buf_pos += sprintf(buf + buf_pos, "%du", rand() % 1000);
}

// 生成随机运算符
static void gen_rand_op() {
    char ops[] = "+-*/";
    gen(ops[rand() % 4]);
}

// 生成随机空格
static void gen_rand_space() {
    if (rand() % 10 == 0) gen(' ');
}

static void gen_rand_expr() {
    gen_rand_space(); // 空格

    if (buf_pos > 60000) {
        gen_num();
        return;
    }

    switch (rand() % 3) {
        case 0: 
            gen_num(); 
            break;
        case 1: 
            gen('('); 
            gen_rand_expr(); 
            gen(')'); 
            break;
        default: 
            gen_rand_expr(); 
            gen_rand_op(); 
            gen_rand_expr(); 
            break;
    }
    
    gen_rand_space(); //空格
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  
  int i;
  for (i = 0; i < loop; i ++) {
    // 重置指针和缓冲区！
    buf_pos = 0;
    buf[0] = '\0'; 

    gen_rand_expr();

    // 加上字符串结束符
    buf[buf_pos] = '\0';

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    // 2> /dev/null 是为了不让 gcc 的报错刷屏
    int ret = system("gcc /tmp/.code.c -o /tmp/.expr 2> /dev/null");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    // 检查 fscanf 的返回值
    // 如果 ret != 1，说明程序崩溃了（除0），没读到结果
    // 坏用例跳过
    if (ret != 1) continue;

    printf("%u %s\n", result, buf);
  }
  return 0;
}
