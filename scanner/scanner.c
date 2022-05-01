/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"
#include <string.h>
#include <limits.h>


extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];
FILE* file;

/***************************************************************/

void skipBlank() {
  while (currentChar != -1 && charCodes[currentChar] == CHAR_SPACE)
	  readChar();
}

void skipComment() {
  while (1) {
    readChar();
    if (currentChar == -1) {
      error(ERR_ENDOFCOMMENT, lineNo, colNo, file);
    } else if (currentChar == 10) {
      readChar();
      return;
    }
  }
}

Token* readIdentKeyword(void) {
  int count = 0;
  Token* token = makeToken(TK_IDENT, lineNo, colNo);

  while (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT || charCodes[currentChar] == CHAR_UNDERSCORE || charCodes[currentChar] == CHAR_DOLAR) {
    token->string[count] = currentChar;
    count++;
    readChar();
  }
  token->string[count] = '\0';

  if (count > MAX_IDENT_LEN) {
    error(ERR_IDENTTOOLONG, lineNo, colNo - count, file);
  } else {
    TokenType type = checkKeyword(token->string);
    if (type != TK_NONE) {
      token->tokenType = type;
    }
  }
  return token;
}

Token* readNumber(void) {
  int count = 0;
  Token* token = makeToken(TK_NUMBER, lineNo, colNo);

  while (charCodes[currentChar] == CHAR_DIGIT) {
    if (count > 9) {
      error(ERR_NUMBERTOOLONG, token->lineNo, token->colNo, file);
    }
    token->string[count] = currentChar;
    count++;
    readChar();
  }
  token->string[count] = '\0';
  token->value = strtol(token->string, 0, 10);
  if(token->value >= INT_MAX){
    error(ERR_NUMBERTOOBIG, token->lineNo, token->colNo, file);
  }

  return token;
}

Token* readConstChar(void) {
  Token* token = makeToken(TK_CHAR, lineNo, colNo);

  readChar();
  if (currentChar == -1) {
    error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo, file);
  } else {
    token->string[0] = currentChar;
      readChar();
      switch(charCodes[currentChar]) {
      case CHAR_SINGLEQUOTE:
        token->string[1] = '\0';
        readChar();
        return token;
      default:
        error(ERR_INVALIDCHARCONSTANT, token->lineNo, token->colNo, file);
        break;
      }
  }
  return token;
}

Token* getToken(void) {
  Token *token;
  int ln, cn;

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);

  switch (charCodes[currentChar]) {
  case CHAR_SPACE: skipBlank(); return getToken();
  case CHAR_LETTER: return readIdentKeyword();
  case CHAR_UNDERSCORE: return readIdentKeyword();
  case CHAR_DIGIT: return readNumber();
  case CHAR_PLUS: 
    token = makeToken(SB_PLUS, lineNo, colNo);
    readChar(); 
    return token;
  case CHAR_MINUS:
    token = makeToken(SB_MINUS, lineNo, colNo);
    readChar();
    return token;
  case CHAR_TIMES:
    token = makeToken(SB_TIMES, lineNo, colNo);
    readChar();
    return token;
  case CHAR_SLASH:
    // token = makeToken(SB_SLASH, lineNo, colNo);
    // readChar();
    // return token;
    token = makeToken(TK_NONE, lineNo, colNo);
    readChar();
    switch(charCodes[currentChar]) {
    case CHAR_SLASH:
      free(token);
      skipComment();
      return getToken();
    default:
      token->tokenType = SB_SLASH;
      return token;
    }
  case CHAR_EQ:
    token = makeToken(SB_EQ, lineNo, colNo);
    readChar();
    return token;
  case CHAR_COMMA:
    token = makeToken(SB_COMMA, lineNo, colNo);
    readChar();
    return token;
  case CHAR_SEMICOLON:
    token = makeToken(SB_SEMICOLON, lineNo, colNo);
    readChar();
    return token;
  case CHAR_RPAR:
    token = makeToken(SB_RPAR, lineNo, colNo);
    readChar();
    return token;
  case CHAR_LPAR:
    token = makeToken(TK_NONE, lineNo, colNo);
    readChar();
    switch(charCodes[currentChar]) {
    case CHAR_PERIOD:
      token->tokenType = SB_LSEL;
      readChar();
      return token;
    case CHAR_TIMES:
      free(token);
      skipComment();
      return getToken();
    case CHAR_SPACE:
      readChar();
      return getToken();
    default:
      token->tokenType = SB_LPAR;
      return token;
    }
  case CHAR_GT:
    token = makeToken(SB_GT, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      token->tokenType = SB_GE;
      readChar();
    }
    return token;
  case CHAR_LT:
    token = makeToken(TK_NONE, lineNo, colNo);

    readChar();
    switch(charCodes[currentChar]) {
    case CHAR_EQ:
      token->tokenType = SB_LE;
      readChar();
      return token;
    default:
      token->tokenType = SB_LT;
      return token;
    }
  case CHAR_EXCLAIMATION:
    token = makeToken(TK_NONE, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] != CHAR_EQ) {
      error(ERR_INVALIDSYMBOL, token->lineNo, token->colNo, file);
    } else {
      token->tokenType = SB_NEQ;
    }
    return token;
  case CHAR_PERIOD:
    token = makeToken(SB_PERIOD, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_RPAR) {
      token->tokenType = SB_RSEL;
      readChar();
    }
    return token;
  case CHAR_COLON:
    token = makeToken(SB_SEMICOLON, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      token->tokenType = SB_ASSIGN;
      readChar();
    }
    return token;
  case CHAR_SINGLEQUOTE:
    return readConstChar();
  default:
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_INVALIDSYMBOL, lineNo, colNo, file);
    readChar(); 
    return token;
  }
}

/******************************************************************/

void printToken(Token *token, FILE* file) {

  fprintf(file, "%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: fprintf(file, "TK_NONE\n"); break;
  case TK_IDENT: fprintf(file, "TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: fprintf(file, "TK_NUMBER(%s)\n", token->string); break;
  case TK_CHAR: fprintf(file, "TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: fprintf(file, "TK_EOF\n"); break;

  case KW_PROGRAM: fprintf(file, "KW_PROGRAM\n"); break;
  case KW_CONST: fprintf(file, "KW_CONST\n"); break;
  case KW_TYPE: fprintf(file, "KW_TYPE\n"); break;
  case KW_VAR: fprintf(file, "KW_VAR\n"); break;
  case KW_INTEGER: fprintf(file, "KW_INTEGER\n"); break;
  case KW_CHAR: fprintf(file, "KW_CHAR\n"); break;
  case KW_ARRAY: fprintf(file, "KW_ARRAY\n"); break;
  case KW_OF: fprintf(file, "KW_OF\n"); break;
  case KW_FUNCTION: fprintf(file, "KW_FUNCTION\n"); break;
  case KW_PROCEDURE: fprintf(file, "KW_PROCEDURE\n"); break;
  case KW_BEGIN: fprintf(file, "KW_BEGIN\n"); break;
  case KW_END: fprintf(file, "KW_END\n"); break;
  case KW_CALL: fprintf(file, "KW_CALL\n"); break;
  case KW_IF: fprintf(file, "KW_IF\n"); break;
  case KW_THEN: fprintf(file, "KW_THEN\n"); break;
  case KW_ELSE: fprintf(file, "KW_ELSE\n"); break;
  case KW_WHILE: fprintf(file, "KW_WHILE\n"); break;
  case KW_DO: fprintf(file, "KW_DO\n"); break;
  case KW_FOR: fprintf(file, "KW_FOR\n"); break;
  case KW_TO: fprintf(file, "KW_TO\n"); break;

  case SB_SEMICOLON: fprintf(file, "SB_SEMICOLON\n"); break;
  case SB_COLON: fprintf(file, "SB_COLON\n"); break;
  case SB_PERIOD: fprintf(file, "SB_PERIOD\n"); break;
  case SB_COMMA: fprintf(file, "SB_COMMA\n"); break;
  case SB_ASSIGN: fprintf(file, "SB_ASSIGN\n"); break;
  case SB_EQ: fprintf(file, "SB_EQ\n"); break;
  case SB_NEQ: fprintf(file, "SB_NEQ\n"); break;
  case SB_LT: fprintf(file, "SB_LT\n"); break;
  case SB_LE: fprintf(file, "SB_LE\n"); break;
  case SB_GT: fprintf(file, "SB_GT\n"); break;
  case SB_GE: fprintf(file, "SB_GE\n"); break;
  case SB_PLUS: fprintf(file, "SB_PLUS\n"); break;
  case SB_MINUS: fprintf(file, "SB_MINUS\n"); break;
  case SB_TIMES: fprintf(file, "SB_TIMES\n"); break;
  case SB_SLASH: fprintf(file, "SB_SLASH\n"); break;
  case SB_LPAR: fprintf(file, "SB_LPAR\n"); break;
  case SB_RPAR: fprintf(file, "SB_RPAR\n"); break;
  case SB_LSEL: fprintf(file, "SB_LSEL\n"); break;
  case SB_RSEL: fprintf(file, "SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token, file);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main()
{
  int menu = 0;
  char string[10];

  do {
    printf("-----MENU-----\n");
    printf("1. Example 3.\n");
    printf("2. End of comment expected!.\n");
    printf("3. Identification too long!.\n");
    printf("4. Number too long!.\n");
    printf("5. Number too big!.\n");
    printf("6. Invalid const char!.\n");
    printf("7. Exit.\n");
    printf("Check for: "); scanf("%d", &menu);

    switch (menu)
    {
    case 1:
      file = fopen("result.txt", "w+");
      strcpy(string, "example3.kpl");
      break;
    case 2:
      file = fopen("result_error1.txt", "w+");
      strcpy(string, "error1.kpl");
      break;
    case 3:
      file = fopen("result_error2.txt", "w+");
      strcpy(string, "error2.kpl");
      break;
    case 4:
      file = fopen("result_error3.txt", "w+");
      strcpy(string, "error3.kpl");
      break;
    case 5:
      file = fopen("result_error4.txt", "w+");
      strcpy(string, "error4.kpl");
      break;
    case 6:
      file = fopen("result_error5.txt", "w+");
      strcpy(string, "error5.kpl");
      break;
    case 7:
      return 0;
    }
    
    if (scan(string) == IO_ERROR) {
      printf("Can\'t read input file!\n");
    }
    fclose(file);
    printf("\n");
  } while (menu != 4);
}