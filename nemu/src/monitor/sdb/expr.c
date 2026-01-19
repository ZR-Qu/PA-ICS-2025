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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_HEX, 
  TK_DEC,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  {" +", TK_NOTYPE},    // spaces

  {"==", TK_EQ},        // equal

  {"\\+", '+'},                 // 匹配 +
  {"\\-", '-'},                 // 匹配 -
  {"\\*", '*'},                 // 匹配 *
  {"\\/", '/'},                 // 匹配 / 
  {"\\(", '('},                 // 匹配 (
  {"\\)", ')'},                 // 匹配 )

  {"0x[0-9a-fA-F]+", TK_HEX},   // 匹配十六进制
  {"[0-9]+", TK_DEC},           // 匹配十进制整数
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0; // Number of Tokens

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        if (rules[i].token_type != TK_NOTYPE && nr_token >= 32) { 
             printf("Error: Too many tokens!\n");
             assert(0);
        }

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_DEC:
          case TK_HEX:
            tokens[nr_token].type = rules[i].token_type;

            if (substr_len >= 32) {
                printf("Error: Token too long at position %d\n", position);
                assert(0);
            }

            // 拷贝
            strncpy(tokens[nr_token].str, substr_start, substr_len);  //不会自动在拷贝结果的末尾追加字符串结束符 \0
            
            // 添加结束符
            tokens[nr_token].str[substr_len] = '\0';

            nr_token++; // 计数器加一
            break;

          default:
            // 对于 +, -, *, /, (, ) 运算符
            // 只需要记录类型，不需要记录具体字符串
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
