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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

#define TEST_EXPR
word_t expr(char *e, bool *success);

// 用于测试expc()
void test_expr() {
    printf("Test: Start testing...\n");
    
    // 打开 input 文件
    FILE *fp = fopen("tools/gen-expr/build/input", "r");
    if (fp == NULL) {
        printf("Error: Cannot open input file 'tools/gen-expr/build/input'.\n");
        printf("Please run 'make' and './gen-expr 10000 > input' in tools/gen-expr/ first.\n");
        return;
    }

    char line[65536];
    int count = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        // input格式； "结果 表达式"
        uint32_t expected_res;
        char *expr_str = line;
        
        if (sscanf(line, "%u", &expected_res) != 1) continue; // 跳过空行
        
        // 指针移动：先指向表达式的开头
        // 比如 "123  1+2"，先跳过 "123"，再跳过空格，指向 "1+2"中的'1'
        while (*expr_str >= '0' && *expr_str <= '9') expr_str++;
        while (*expr_str == ' ') expr_str++; 

        bool success;
        word_t actual_res = expr(expr_str, &success);

        if (success == false) {
            printf("\n[Error] expr failed (syntax error?)\n");
            printf("Line %d: %s\n", count, expr_str);
            assert(0);
        }

        if (actual_res != expected_res) {
            printf("\n[Error] Test failed!\n");
            printf("Line:     %d\n", count);
            printf("Expr:     %s\n", expr_str);
            printf("Expected: %u\n", expected_res);
            printf("Actual:   %u\n", actual_res);
            assert(0);
        }
        
        count++;

        if (count % 1000 == 0) {
            printf("Passed %d tests...\n", count);
        }
    }

    printf("\n\033[1;32mCongratulation! passed all %d tests!\033[0m\n", count);
    fclose(fp);
}

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

#ifdef TEST_EXPR  // 用于测试expc()
  test_expr();
  return 0;
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
