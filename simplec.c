//================================================================================================//
//                                       Nikolai Korolkov
//                                       SimpleC compiler
//                                        Fall 2019, UCF
//================================================================================================//

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define PROJ4_PROLOGUE \
"target triple = \"x86_64-pc-linux-gnu\"\n" \
"\n" \
"declare i32 @printf(i8*, ...) #1\n" \
"@.str = private unnamed_addr constant [4 x i8] c\"\%d\\0A\\00\", align 1\n" \
"\n" \
"define void @print_integer(i32) #0 {\n" \
"  \%2 = alloca i32, align 4\n" \
"  store i32 \%0, i32* \%2, align 4\n" \
"  \%3 = load i32, i32* \%2, align 4\n" \
"  \%4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 \%3)\n" \
"  ret void\n" \
"}\n" \
"\n" \
"\%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, \%struct._IO_marker*, \%struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, \%struct._IO_codecvt*, \%struct._IO_wide_data*, \%struct._IO_FILE*, i8*, i64, i32, [20 x i8] }\n" \
"\%struct._IO_marker = type opaque\n" \
"\%struct._IO_codecvt = type opaque\n" \
"\%struct._IO_wide_data = type opaque\n" \
"\n" \
"@stderr = external dso_local global \%struct._IO_FILE*, align 8\n" \
"@.str.1 = private unnamed_addr constant [25 x i8] c\"please enter an integer\\0A\\00\", align 1\n" \
"@.str.2 = private unnamed_addr constant [3 x i8] c\"\%d\\00\", align 1\n" \
"@.str.3 = private unnamed_addr constant [6 x i8] c\"scanf\\00\", align 1\n" \
"@.str.4 = private unnamed_addr constant [24 x i8] c\"no matching characters\\0A\\00\", align 1\n" \
"\n" \
"declare i32* @__errno_location() #2\n" \
"declare i32 @__isoc99_scanf(i8*, ...) #1\n" \
"declare void @perror(i8*) #1\n" \
"declare void @exit(i32) #3\n" \
"declare i32 @fprintf(\%struct._IO_FILE*, i8*, ...) #1\n" \
"\n" \
"define i32 @read_integer() #0 {\n" \
"  \%1 = alloca i32, align 4\n" \
"  \%2 = alloca i32, align 4\n" \
"  \%3 = call i32* @__errno_location() #4\n" \
"  store i32 0, i32* \%3, align 4\n" \
"  \%4 = load \%struct._IO_FILE*, \%struct._IO_FILE** @stderr, align 8\n" \
"  \%5 = call i32 (\%struct._IO_FILE*, i8*, ...) @fprintf(\%struct._IO_FILE* \%4, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.1, i32 0, i32 0))\n" \
"  \%6 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.2, i32 0, i32 0), i32* \%1)\n" \
"  store i32 \%6, i32* \%2, align 4\n" \
"  \%7 = load i32, i32* \%2, align 4\n" \
"  \%8 = icmp eq i32 \%7, 1\n" \
"  br i1 \%8, label \%9, label \%11\n" \
"\n" \
"; <label>:9:                                      ; preds = \%0\n" \
"  \%10 = load i32, i32* \%1, align 4\n" \
"  ret i32 \%10\n" \
"\n" \
"; <label>:11:                                     ; preds = \%0\n" \
"  \%12 = call i32* @__errno_location() #4\n" \
"  \%13 = load i32, i32* \%12, align 4\n" \
"  \%14 = icmp ne i32 \%13, 0\n" \
"  br i1 \%14, label \%15, label \%16\n" \
"\n" \
"; <label>:15:                                     ; preds = \%11\n" \
"  call void @perror(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.3, i32 0, i32 0))\n" \
"  call void @exit(i32 1) #5\n" \
"  unreachable\n" \
"\n" \
"; <label>:16:                                     ; preds = \%11\n" \
"  \%17 = load \%struct._IO_FILE*, \%struct._IO_FILE** @stderr, align 8\n" \
"  \%18 = call i32 (\%struct._IO_FILE*, i8*, ...) @fprintf(\%struct._IO_FILE* \%17, i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str.4, i32 0, i32 0))\n" \
"  call void @exit(i32 1) #5\n" \
"  unreachable\n" \
"}\n" \
"\n"

// Counting LLVM variables
int variablesCounter = 1;
int conditionsCounter = 1;
int jumpCounter = 1;
int exitCounter = 1;
int error = 0;

// Linked list symbol table structure
typedef struct SymbolTable
{
  char *name;
  int adress;
  struct SymbolTable *next;
}SymbolTable;

// Stuck structure
typedef struct StackForPostfix
{
  int size;
  int top;
  int *data;
}StackForPostfix;

typedef struct functionsList
{
   char *name;
   SymbolTable *symbolTable;
   struct functionsList *next;
}functionsList;

// Expression Tree (types: 1 - operator, 2 - operand)
// Data types: 0 - integer, 1 - boolean
typedef struct ExpTree
{
  int type;
  int dataType;
  int negation;
  char *data;
  int llvmVar;
  struct ExpTree *left;
  struct ExpTree *right;
}ExpTree;

// PROTOTYPES:
char *infixToPostfix(char expr[], SymbolTable *root, functionsList *fnList);
ExpTree *postfixToTree(char *postfix, SymbolTable *symbolTable);
void SolveTree(ExpTree *root);
int expressionIsConstant(char *expr);

functionsList *fnListAdd(functionsList *root, char *name, SymbolTable *symbolTable)
{
   functionsList *node = NULL;
   functionsList *walk = NULL;

   node = calloc(1, sizeof(functionsList));
   node->name = name;
   node->symbolTable = symbolTable;
   node->next = NULL;

   if (root == NULL)
      return node;

   walk = root;


   while(walk->next != NULL)
      walk = walk->next;

   walk->next = node;

   return root;
}

// Returns 1 if found
int fnListContains(functionsList *root, char *name)
{
   if (root == NULL)
      return 0;

   functionsList *walk = NULL;

   walk = root;

   while (walk != NULL)
   {
      if (strcmp(walk->name, name) == 0)
         return 1;
      walk = walk->next;
   }

   return 0;
}

int parser(char *input, SymbolTable *st, functionsList *fnList);

// Returns string assiciated with integer
char *intToString(int n)
{
   char *res = NULL;
   int tmp = n;
   int len = 0;
   int i;

   while (tmp != 0)
   {
      tmp /= 10;
      len++;
   }



   tmp = n;
   res = calloc(len + 1, sizeof(char));
   for (i = len - 1; i >= 0; i--)
   {
      res[i] = (tmp % 10) + '0';
      tmp /= 10;
   }
   res[len] = '\0';

   return res;
}

// Debugging function
void printSymbolTable(SymbolTable *root)
{
   SymbolTable *tmp = root;
   if (root == NULL)
   {
      printf("EMPTY TABLE\n");
      return;
   }
   while(tmp != NULL)
   {
      printf("%s@%d -> ", tmp->name, tmp->adress);
      tmp = tmp->next;
   }
   printf("\n");
}

// Freeing SymbolTable
void freeSymbolTable(SymbolTable *root)
{
   if (root == NULL)
      return;
   freeSymbolTable(root->next);
   free(root->name);
   root->name = NULL;
   free(root);
   root = NULL;
}

SymbolTable *cloneSymbolTable(SymbolTable *source)
{
   SymbolTable *clone = NULL;
   SymbolTable *node = NULL;
   SymbolTable *tmp = source;
   int i, len;

   while (tmp != NULL)
   {
      node = calloc(1, sizeof(SymbolTable));
      len = strlen(tmp->name);
      node->name = calloc(len + 1, sizeof(char));
      node->name[len] = '\0';

      for (i = 0; i < len; i++)
         node->name[i] = tmp->name[i];
      node->adress = tmp->adress;

      if (clone == NULL)
         clone = node;
      else
         node = node->next;

      tmp = tmp->next;
   }

   // printf("Source:\n");
   // printSymbolTable(source);
   // printf("Clone:\n");
   // printSymbolTable(clone);

   return clone;
}

// Returns adress if variable found, 0 otherwise
int symbolTableContains(SymbolTable *root, char *name)
{
   SymbolTable *tmp = root;
   if (root == NULL)
      return 0;
   while (tmp != NULL)
   {
      if (strcmp(tmp->name, name) == 0)
         return tmp->adress;
      tmp = tmp->next;
   }
   return 0;
}

SymbolTable *symbolTableAdd(SymbolTable *root, char *name, int adress)
{
   SymbolTable *node = calloc(1, sizeof(SymbolTable));
   SymbolTable *walk = NULL;

   node->name = name;
   node->adress = adress;
   node->next = NULL;

   if (root == NULL)
      return node;

   walk = root;
   while (walk->next != NULL)
      walk = walk->next;

   walk->next = node;

   return root;
}

// returns 0 if not the sequence or position to jump otherwise
int checkSequence(char *input, int position, int inputLen, char sequence[])
{
   int i = 0;
   int sequenceLen = strlen(sequence);

   if (position + sequenceLen >= inputLen)
      return 0;

   for (i = 0; i < sequenceLen; i++)
      if (sequence[i] != input[position+i])
         return 0;

   return position + sequenceLen;
}

// returns NULL if not the assignment or variable adress to assign
int checkAssignmentSequence(SymbolTable *root, char *input, int position, int inputLen)
{
   if (root == NULL)
      return 0;
   int i, jump = 0;
   char *sequence;
   SymbolTable *tmp = root;

   while (tmp != NULL)
   {
      sequence = calloc(strlen(tmp->name) + 2, sizeof(char));
      strcat(sequence, tmp->name);
      strcat(sequence, " =");
      if (jump = checkSequence(input, position, inputLen, sequence))
         break;
      strcpy(sequence, tmp->name);
      strcat(sequence, "=");
      if (jump = checkSequence(input, position, inputLen, sequence))
         break;
      free(sequence);
      tmp = tmp->next;
   }

   if (tmp == NULL)
      return 0;
   else
   {
      free(sequence);
      return tmp->adress;
   }
}

// returns int value of operator's precedence in context of our grammar
int getPrecedence(char c)
{
  // Precedence:
  // 7     !
  // 6   * / %
  // 5   + -
  // 4   > <
  // 3   == !=
  // 2   &&
  // 1   ||

   // !  -----> ! in stack
   // >  -----> > in stack
   // <  -----> < in stack
   // == -----> = in stack
   // != -----> ~ in stack
   // && -----> & in stack
   // || -----> | in stack

   if (c == '!')
      return 7;
   else if (c == '/' || c == '*' || c == '%')
      return 6;
   else if (c == '+' || c == '-')
      return 5;
   else if (c == '>' || c == '<')
      return 4;
   else if (c == '=' || c == '~')
      return 3;
   else if (c == '&')
      return 2;
   else if (c == '|')
      return 1;

   return 666;
}

// Push 1 value on top of the stack
void push(StackForPostfix *stack, int value)
{
  stack->size++;
  stack->top++;
  stack->data[stack->top] = value;
}

// returns char pointer of a value on top of the StackForPostfix
// and modify stack as necessary for popChar operation
char *popChar(StackForPostfix *stack)
{
  char *res = calloc(1, sizeof(char));
  if (stack->size == 0)
    printf("popChar on empty stack\n");
  *res = (char)(stack->data[stack->top]);
  stack->top--;
  stack->size--;

  return res;
}

void llvmDeclaration(int adress)
{
   printf("  %ct%d = alloca i32\n", '%', adress);
}

void llvmAssignment(char *var1, char *var2, char operator, char *var3)
{
   printf("  %ct%s = ", '%', var1);

   if (operator == '+')
     printf("add nsw i32 ");
   else if (operator == '-')
     printf("sub nsw i32 ");
   else if (operator == '*')
     printf(" mul nsw i32 ");
   else if (operator == '/')
     printf("sdiv i32 ");
   else if (operator == '%')
     printf("srem i32 ");
   else if (operator == '<')
      printf("icmp slt i32 ");
   else if (operator == '>')
      printf("icmp sgt i32 ");
   else if (operator == '=')
      printf("icmp eq i32 ");
   else if (operator == '~')
      printf("icmp ne i32 ");
   else if (operator == '&')
      printf("and i1 ");
   else if (operator == '|')
      printf("or i1 ");
   else if (operator == 'X')
      printf("xor i1 ");


   printf("%s, %s\n", var2, var3);
}

void llvmAssignmentToConstant(int adress, int constant)
{
   printf("  store i32 %d, i32* %ct%d\n", constant, '%', adress);
}

void llvmAssignmentToVar(int adress, char *var)
{
   printf("  store i32 %s, i32* %ct%d\n", var, '%', adress);
}

// Returns variable containing value of llvm variable
char *llvmLoad(char *var)
{
   char *res = NULL;
   char *vc = intToString(variablesCounter);
   printf("  %ct%d = load i32, i32* %s\n", '%', variablesCounter, var);
   variablesCounter++;
   res = calloc(3 + strlen(vc), sizeof(char));
   strcat(res, "%t");
   strcat(res, vc);
   res[2+strlen(vc)] = '\0';

   return res;
}

void llvmReadInteger(int adress)
{
   char *var = NULL;
   char *res2 = NULL;

   printf("  %ct%d = call i32 @read_integer()\n", '%', variablesCounter);
   res2 = intToString(variablesCounter);
   var = calloc(3 + strlen(res2), sizeof(char));
   strcpy(var, "%t");
   strcat(var, res2);
   var[strlen(var)] = '\0';

   variablesCounter++;
   llvmAssignmentToVar(adress, var);

   if (var != NULL)
      free(var);
   if (res2 != NULL)
      free(res2);
}

void llvmPrintVariable(char *var)
{
  printf("  call void @print_integer(i32 %s)\n", var);
}

void llvmPrintConstant(int k)
{
  printf("  call void @print_integer(i32 %d)\n", k);
}

void llvmCondIfThenElse(int condNum)
{
   // br i1 %cond, label %ifbranch2, %elsebranch2

   printf("  br i1 %%t%d, label %%jump%d, label %%jump%d\n", condNum, jumpCounter, jumpCounter + 1);
   jumpCounter += 2;
}

void llvmIf(int condNum)
{
   printf("  br i1 %%t%d, label %%jump%d, label %%exit%d\n", condNum, jumpCounter, exitCounter);
   jumpCounter++;
   exitCounter++;
}

void llvmWhile(int condNum, int jumpBody, int jumpExit)
{
   printf("  ; jump to body or to exit\n");
   printf("  br i1 %%t%d, label %%jump%d, label %%exit%d\n", condNum, jumpBody, jumpExit);
}

void llvmDefineFunction(char *name, int numParameters)
{
   int i;

   printf("define i32 @%s(", name);
   if (numParameters == 0)
      printf(")\n{\n");

   for (i = 0; i < numParameters; i++)
   {
      printf("i32");
      if (i + 1 != numParameters)
         printf(", ");
      else
         printf(")\n{\n");
   }

   if (numParameters != 0)
      printf("  ; Parameters:\n");

   for (i = 1; i < numParameters + 1; i++)
   {
      printf("  %%t%d = alloca i32\n", i);
      printf("  store i32 %%%d, i32* %%t%d, align 4\n", i - 1, i);
   }
   printf("\n  ; Function body:\n");
}

// Returns adress of variable containing
int llvmFunctionCall(char *name, char *param, SymbolTable *symbolTable, functionsList *fnList)
{
   int address;
   int paramLen = strlen(param);
   int numParam = 1;
   int *paramArr = calloc(20, sizeof(int));
   int index = 0;
   int i = 0;
   int j = 0;
   char *postfix = NULL;
   ExpTree *eTree = NULL;

   // allocating new variable
   printf("  %%t%d = alloca i32\n", variablesCounter);
   address = variablesCounter;
   symbolTableAdd(symbolTable, intToString(address), address);
   variablesCounter++;

   // If function has no parameters:
   if (strcmp(param, "") == 0)
   {
      printf("  %%t%d = call i32 @%s()\n", variablesCounter, name);
      variablesCounter++;
      printf("  store i32 %%t%d, i32* %%t%d, align 4\n", variablesCounter - 1, address);
      return address;
   }

   for (i = 0; i < paramLen; i++)
   {
      if (param[i] == ',')
         numParam++;
   }

   // If a function has 1 parameter:
   if (numParam == 1)
   {
      // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
      // printf(">>>llvmFunctionCall:\n>>>>>Param: '%s'\n", param);

      // Generating condition code
      postfix = infixToPostfix(param, symbolTable, fnList);
      if (expressionIsConstant(postfix))
      {
         printf("  %%t%d = call i32 @%s(i32 %d)\n", variablesCounter, name, atoi(postfix));
         variablesCounter++;
         printf("  store i32 %%t%d, i32* %%t%d, align 4\n", variablesCounter - 1, address);
         variablesCounter++;
      }
      else
      {
         eTree = postfixToTree(postfix, symbolTable);
         SolveTree(eTree);
         if (eTree->llvmVar == 1)
         {
            printf("  %%t%d = load i32, i32* %s\n", variablesCounter, eTree->data);
            variablesCounter++;
         }

         printf("  %%t%d = call i32 @%s(i32 %%t%d)\n", variablesCounter, name, variablesCounter - 1);
         variablesCounter++;
         printf("  store i32 %%t%d, i32* %%t%d, align 4\n", variablesCounter - 1, address);
         variablesCounter++;

         return address;
      }
      // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
   }
   // If function has more parameters:
   else
   {
      // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
      for (i = 0; i < paramLen; i++)
      {
         if (param[i] != ',' && i != paramLen - 1)
            continue;

         if (param[i] == ',')
         {
            param[i] = '\0';

            postfix = infixToPostfix(param, symbolTable, fnList);
            if (expressionIsConstant(postfix))
            {
               printf("  %%t%d = add nsw i32 0, %d\n", variablesCounter, atoi(postfix));
               variablesCounter++;
               paramArr[index] = variablesCounter - 1;
               index++;
            }
            else
            {
               eTree = postfixToTree(postfix, symbolTable);
               SolveTree(eTree);

               if (eTree->llvmVar == 1)
               {
                  printf("  %%t%d = load i32, i32* %s\n", variablesCounter, eTree->data);
                  variablesCounter++;
               }

               paramArr[index] = variablesCounter - 1;
               index++;
            }

            // Cleaning first part of param
            for (j = 0; j <= i; j++)
               param[j] = ' ';
         }
         else
         {
            postfix = infixToPostfix(param, symbolTable, fnList);
            if (expressionIsConstant(postfix))
            {
               printf("  %%t%d = add nsw i32 0, %d\n", variablesCounter, atoi(postfix));
               variablesCounter++;
               paramArr[index] = variablesCounter - 1;
               index++;
            }
            else
            {
               eTree = postfixToTree(postfix, symbolTable);
               SolveTree(eTree);
               if (eTree->llvmVar == 1)
               {
                  printf("  %%t%d = load i32, i32* %s\n", variablesCounter, eTree->data);
                  variablesCounter++;
               }
               paramArr[index] = variablesCounter - 1;
               index++;
            }

         }
      }

      printf("  %%t%d = call i32 @%s(", variablesCounter, name);
      variablesCounter++;

      for (i = 0; i < numParam; i++)
      {
         if (i != numParam - 1)
            printf("i32 %%t%d, ", paramArr[i]);
         else
            printf("i32 %%t%d)\n", paramArr[i]);
      }
      printf("  store i32 %%t%d, i32* %%t%d, align 4\n", variablesCounter - 1, address);
      // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
   }

   free(paramArr);
   paramArr = NULL;
   return address;
}

// returns 1 if expression is constant, zero otherwise
int expressionIsConstant(char *expr)
{
  int i, len = strlen(expr);

  if (expr[len-1] == ' ')
   len--;

  if (len == 1 && expr[0] == '-')
   return 0;

  for(i = 0; i < len; i++)
  {
    if (i == 0 && expr[i] == '-')
      continue;

    if (!isdigit(expr[i]))
      return 0;
  }

  return 1;
}

char *infixToPostfix(char expr[], SymbolTable *root, functionsList *fnList)
{
  StackForPostfix *stack = NULL;
  char *postfix = NULL;
  char *c = NULL;
  char *str = NULL;
  char *varName = NULL;
  char *parameters = NULL;
  int oldPosition, counter, index;
  int i, len = strlen(expr);
  int adress = 0;
  int negativeNumber = 0;
  int numLP, numRP;


  stack = malloc(1 * sizeof(StackForPostfix));
  stack->data = malloc(len * sizeof(int));
  stack->top = -1;
  stack->size = 0;
  postfix = calloc(25 * len, sizeof(char));
  c = calloc(1, sizeof(char));

  for (i = 0; i < len; i++)
  {
    // printf("~> %c\n", expr[i]);
    // printf("postfix: %s\n", postfix);
    // if number token is detected concatenating it to the end of the postfix string
    if (isdigit(expr[i]))
    {
      if (negativeNumber)
      {
        strcat(postfix, "-");
        negativeNumber = 0;
      }

      index = 0;
      while (i < len && isdigit(expr[i]))
      {
        *c = expr[i];
        strcat(postfix, c);

        i++;
      }
      strcat(postfix, " ");

      if (i >= len)
        break;
      i--;
      continue;
    }
    // if LP detected push it on stack
    if (expr[i] == '(')
    {
      push(stack, '(');
      continue;
    }
    // if RP detected popChar from stack to the resulting postfix string
    // util LP is on top, then delete LP from stack
    else if (expr[i] == ')')
    {
      while (stack->data[stack->top] != '(')
      {
        c = popChar(stack);
        strcat(postfix, c);
        strcat(postfix, " ");
      }
         popChar(stack);
      continue;
    }
    // if other operand is detected, popChar from the stack to the resulting postfix string
    // until: stack is not empty or LP is detected or lower precedence is detected
    else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/' || expr[i] == '%')
    {
      // Checking if MINUS is a part of number:
      // if minus is 1 token in expression
      if (i == 0 && expr[i] == '-' && i + 1 < len && isdigit(expr[i+1]))
      {
        negativeNumber = 1;
        continue;
      }
      else if ((i > 0) && (expr[i] == '-') && (!isdigit(expr[i-1])) && (!isalpha(expr[i-1]))
               && (expr[i-1] != ')') && (i + 1 < len) && isdigit(expr[i+1]))
      {
        negativeNumber = 1;
        continue;
      }

      while(stack->size != 0 && stack->data[stack->top] != '(' && getPrecedence(expr[i]) <= getPrecedence(stack->data[stack->top]))
      {
        c = popChar(stack);
        strcat(postfix, c);
        strcat(postfix, " ");
      }
      push(stack, expr[i]);
    }
    // If comparison or logic operand may be detected
    else if (i + 1 < len && !isalpha(expr[i]))
    {
      // !  -----> ! in stack +
      // >  -----> > in stack +
      // <  -----> < in stack +
      // == -----> = in stack +
      // != -----> ~ in stack
      // && -----> & in stack
      // || -----> | in stack

      // Indicating that c unused
      *c = '#';
      oldPosition = i;

      if (expr[i] == '!' && expr[i+1] != '=')
      {
         *c = '!';
      }
      else if (expr[i] == '>')
      {
         *c = '>';
      }
      else if (expr[i] == '<')
      {
         *c = '<';
      }
      else if (expr[i] == '=' && expr[i+1] == '=')
      {
         *c = '=';
         i++;
      }
      else if (expr[i] == '!' && expr[i+1] == '=')
      {
         *c = '~';
         i++;
      }
      else if (expr[i] == '&' && expr[i+1] == '&')
      {
         *c = '&';
         i++;
      }
      else if (expr[i] == '|' && expr[i+1] == '|')
      {
         *c = '|';
         i++;
      }

      if (*c != '#')
      {
         while(stack->size != 0 && stack->data[stack->top] != '(' && getPrecedence(*c) <= getPrecedence(stack->data[stack->top]))
         {
            // c = popChar(stack);
            strcat(postfix, popChar(stack));
            strcat(postfix, " ");
         }
         push(stack, *c);
         continue;
      }
      else
      {
         i = oldPosition;
      }
    }
    // Checking for variable or function
    else
    {
      // Buffering name:
      {
         oldPosition = i;
         counter = 0;
         index = 0;

         while (i < len && (isalpha(expr[i]) || isdigit(expr[i])))
         {
            counter++;
            i++;
         }
         varName = calloc(counter, sizeof(char));

         i = oldPosition;
         index = 0;

         while (i < len && (isalpha(expr[i]) || isdigit(expr[i])))
         {
            varName[index] = expr[i];
            i++;
            index++;
         }
         varName[index] = '\0';
      }

      // Checking if variable declared
      adress = symbolTableContains(root, varName);

      // If function undeclared: error
      if (!fnListContains(fnList, varName) && expr[i] == '(')
      {
         fprintf(stderr, "error: use of undeclared function %s\n", varName);
         error = 1;
         return NULL;
      }
      // If variable undeclared
      else if (!adress && !fnListContains(fnList, varName))
      {
         fprintf(stderr, "error: use of undeclared variable %s\n", varName);
         error = 1;
         return NULL;
      }

      // If variable caught:
      if (adress)
      {
         str = intToString(adress);

         strcat(postfix, "%t");
         strcat(postfix, str);
         strcat(postfix, " ");


         free(str);
         str = NULL;


         free(varName);
         varName = NULL;

         i--;
      }
      // If function caught:
      else
      {
         // Buffering parameters:
         {
            index = 0;
            counter = 0;
            numLP = 0;
            numRP = 0;

            oldPosition = i;

            while (numLP == 0 || (numLP != numRP))
            {
               if (expr[i] == '(')
                  numLP++;
               else if (expr[i] == ')')
                  numRP++;

               counter++;
               i++;
            }

            if (counter == 0)
            {
               // function call with no parameters:
               adress = llvmFunctionCall(varName, "", root, fnList);
               strcat(postfix, "%t");
               strcat(postfix, intToString(adress));
               strcat(postfix, " ");
               continue;
            }

            parameters = calloc(counter - 1, sizeof(char));
            parameters[counter-2] = '\0';

            i = oldPosition + 1;
            while (counter != 2)
            {
               parameters[index] = expr[i];
               index++;
               i++;
               counter--;
            }


         }
         // Getting result of function call at (variablesCounter - 1) register:
         adress = llvmFunctionCall(varName, parameters, root, fnList);

         strcat(postfix, "%t");
         strcat(postfix, intToString(adress));
         strcat(postfix, " ");
      }

      continue;
    }
   }

  // popChar the rest of stuck in resulting postfix string
  while(stack->size != 0)
  {
    c = popChar(stack);
    strcat(postfix, c);
    strcat(postfix, " ");
  }
  postfix[strlen(postfix)] = '\0';

  if (stack->data != NULL)
   free(stack->data);
  stack->data = NULL;

  if (stack != NULL)
   free(stack);
  stack = NULL;

  if (c != NULL)
  {
    free(c);
    c = NULL;
  }
  if (str != NULL)
  {
     free(str);
     str = NULL;
  }

  return postfix;
}

ExpTree *createTreeNode(char *data, int type, int dataType, int negation, SymbolTable *symbolTable)
{
   int checkAdress;
   SymbolTable *tmp = symbolTable;
   ExpTree *node = calloc(1, sizeof(ExpTree));
   node->type = type;
   node->dataType = dataType;
   node->negation = negation;
   node->data = data;

   if (strlen(data) > 2 && data[0] == '%' && data[1] == 't')
   {
      // // check table for adress
      // checkAdress = atoi(data + 2);
      //
      // while (tmp != NULL)
      // {
      //    if (tmp->adress == checkAdress)
      //    {
      //       node->llvmVar = 1;
      //       break;
      //    }
      //    tmp = tmp->next;
      // }
      node->llvmVar = 1;
   }

   return node;
}

// Insertion in expression tree
ExpTree *treeInsert(ExpTree *root, char *data, int type, int dataType, int negation, int *done, SymbolTable *symbolTable)
{
   if (*done == 1)
      return root;
   if (root == NULL)
   {
      *done = 1;
      return createTreeNode(data, type, dataType, negation, symbolTable);
   }
   if (root->type == 2)
      return root;


   root->right = treeInsert(root->right, data, type, dataType, negation, done, symbolTable);
   root->left = treeInsert(root->left, data, type, dataType, negation, done, symbolTable);

   return root;
}

void SolveTree(ExpTree *root)
{
   if (error)
      return;
   if (root == NULL)
      return;
   if (root->left == NULL && root->right == NULL)
      return;
   char *vc = NULL;
   char *newData = NULL;
   char operator;

   // Recursive calls
   SolveTree(root->left);
   SolveTree(root->right);

   // If operator at root (int, int) -> int
   if (root->type == 1 && root->dataType == 0)
   {
      // Type checking
      if (root->left->dataType != 0 || root->right->dataType != 0)
      {
         fprintf(stderr, "error: type-checking: integer operator cannot have boolean operand\n");
         error = 1;
         return;
      }
      // Checking if load instruction needed:
      if (root->left->llvmVar == 1)
      {
         newData = llvmLoad(root->left->data);
         free(root->left->data);
         root->left->data = newData;
      }
      if (root->right->llvmVar == 1)
      {
         newData = llvmLoad(root->right->data);
         free(root->right->data);
         root->right->data = newData;
      }

      operator = root->data[0];
      free(root->data);

      vc = intToString(variablesCounter);
      root->data = calloc(3 + strlen(vc), sizeof(char));
      root->data[strlen(vc)+2] = '\0';
      strcpy(root->data, "%t");
      strcat(root->data, vc);

      // Code generation
      llvmAssignment(vc, root->left->data, operator, root->right->data);
      variablesCounter++;
      free(root->left->data);
      free(root->right->data);
      free(root->left);
      free(root->right);
   }
   // If operator ((int, int)||(boolean, boolean)) -> boolean
   else if (root->type == 1 && root->dataType == 1)
   {
      // Type checking
      if (root->left->dataType != 1 || root->right->dataType != 1)
      {
         if (root->data[0] == '>' || root->data[0] == '<' || root->data[0] == '~' || root->data[0] == '=')
         {
            if (root->left->dataType != 0 || root->right->dataType != 0)
            {
               fprintf(stderr, "error: type-checking: comparison operators cannot have boolean operand\n");
               error = 1;
               return;
            }
         }
         else
         {
            fprintf(stderr, "error: type-checking: logical operator cannot have integer operand\n");
            error = 1;
            return;
         }
      }
      // Checking if load instruction needed:
      if (root->left->llvmVar == 1)
      {
         newData = llvmLoad(root->left->data);
         free(root->left->data);
         root->left->data = newData;
      }
      if (root->right->llvmVar == 1)
      {
         newData = llvmLoad(root->right->data);
         free(root->right->data);
         root->right->data = newData;
      }

      operator = root->data[0];
      free(root->data);

      vc = intToString(variablesCounter);
      root->data = calloc(3 + strlen(vc), sizeof(char));
      root->data[strlen(vc)+2] = '\0';
      strcpy(root->data, "%t");
      strcat(root->data, vc);

      // Code generation
      llvmAssignment(vc, root->left->data, operator, root->right->data);
      variablesCounter++;

      // If negation detected
      if (root->negation)
      {
         if (vc != NULL)
            free(vc);

         operator = 'X';
         vc = intToString(variablesCounter);
         llvmAssignment(vc, root->data, operator, "1");

         free(root->data);
         root->data = NULL;
         root->data = calloc(3 + strlen(vc), sizeof(char));
         root->data[strlen(vc)+2] = '\0';

         strcpy(root->data, "%t");
         strcat(root->data, vc);

         variablesCounter++;
      }

      free(root->left->data);
      free(root->right->data);
      free(root->left);
      free(root->right);


   }

   if (vc != NULL)
      free(vc);
}

void traversal(ExpTree *root)
{
   if (root == NULL)
      return;
   traversal(root->left);
   printf("%s ", root->data);
   traversal(root->right);
}

void forestFire(ExpTree *root)
{
   if (root == NULL)
      return;

   forestFire(root->left);
   forestFire(root->right);

   if (root->data != NULL)
   {
      free(root->data);
      root->data = NULL;
   }
   if (sizeof(root) < sizeof(ExpTree))
      root = NULL;
   else
      free(root);
}

ExpTree *postfixToTree(char *postfix, SymbolTable *symbolTable)
{
   if (postfix == NULL)
      return NULL;

   ExpTree *root = NULL;
   int type;
   int i, len = strlen(postfix);
   int jump;
   int counter = 0;
   int index = 0;
   char *data = NULL;
   int *done = calloc(1, sizeof(int));
   int negation = 0;
   int dataType = 0;


   // Reading postfix backwards and inserting every value into the three
   for (i = len - 1; i >= 0; i--)
   {
      dataType = 0;
      counter = 0;
      index = 0;

      if (postfix[i] == ' ')
         i--;

      while (postfix[i] != ' ' && i != -1)
      {
         i--;
         counter++;
      }
      jump = i;
      i++;

      data = calloc(counter + 1, sizeof(char));
      data[counter] = '\0';
      while (postfix[i] != ' ')
      {
         data[index++] = postfix[i];
         i++;
      }

      // Determining types (Operator 1 or Operand 2)
      if (expressionIsConstant(data) || ((strlen(data) > 2) && data[0] == '%' && data[1] == 't'))
      {
         type = 2;
      }
      else
      {
         type = 1;
         // Evaluating data types:
         if (strlen(data) == 1)
         {
            if (data[0] == '!')
            {
               negation = 1;
               // Jumping to the next postfix chunk
               i = jump;
               continue;
            }
            // Checking dataType boolean - 1 int - 2
            else if (data[0] == '>' || data[0] == '<' || data[0] == '=' || data[0] == '~' || data[0] == '&' || data[0] == '|')
               dataType = 1;
         }
      }

      // Type-checking for negation
      if (negation && (type == 2 || dataType == 0))
      {
         error = 1;
         fprintf(stderr, "error: type-checking: negation cannot be applied to an integer\n");
         return root;
      }
      *done = 0;
      root = treeInsert(root, data, type, dataType, negation, done, symbolTable);
      negation = 0;

      // Jumping to the next postfix chunk
      i = jump;
   }

   return root;
}

// Returns input file as char array
char *getInputAsCharArray(char fileName[])
{
   FILE *fp = NULL;
   char *input = NULL;
   char *c = NULL;
   int inputLen = 0;


   // Getting input length
   c = calloc(1, sizeof(char));
   fp = fopen(fileName, "r");
   if (fp == NULL)
   {
      fprintf(stderr, "error: %s: No such file or directory\n", fileName);
      error = 1;
      return NULL;
   }

   while (*c != EOF)
   {
      *c = fgetc(fp);
      inputLen++;
   }
   fclose(fp);


   // Allocating space for input char array
   input = calloc(inputLen + 1, sizeof(char));
   *c = '~';
   fp = fopen(fileName, "r");
   if (fp == NULL)
   {
      fprintf(stderr, "error: %s: No such file or directory\n", fileName);
      error = 1;
      return NULL;
   }

   // Copying input from file
   while (*c != EOF)
   {
      *c = fgetc(fp);
      strcat(input, c);
   }

   // Garbage collection
   fclose(fp);
   free(c);
   return input;
}

void ifThenElseStatement(char cond[], char ifb[], char elseb[], SymbolTable *symt, functionsList *fnList)
{
   char *postfx = NULL;
   ExpTree *eTree = NULL;
   int jump1;
   int jump2;
   int exitCurrent = exitCounter;

   // printf("\n===============SimpleC============\n");
   // printf("condition>\n'%s'\n", cond);
   // printf("ifBody>   \n'%s'\n", ifb);
   // printf("elseBody> \n'%s'\n", elseb);
   // printf("===================================\n");
   // printSymbolTable(symt);

   // getting condition value
   postfx = infixToPostfix(cond, symt, fnList);
   eTree = postfixToTree(postfx, symt);
   SolveTree(eTree);

   printf("\n  ; if %s\n", cond);

   // ifthenelse
   if (elseb != NULL)
   {
      // Code Generation for condition
      llvmCondIfThenElse(variablesCounter - 1);
      jump1 = jumpCounter - 2;
      jump2 = jumpCounter - 1;
      exitCounter++;

      // Code Generation for ifBranch label:
      printf("jump%d:\n", jump1);
      // *~~~~~~~~~Code~~~~~~~~~~~*
      parser(ifb, symt, fnList);
      // *~~~~~~~~~Code~~~~~~~~~~~*
      printf("  br label %%exit%d\n", exitCurrent);

      // Code Generation for ElseBranch label:
      printf("jump%d:\n", jump2);
      // *~~~~~~~~~Code~~~~~~~~~~~*
      parser(elseb, symt, fnList);
      // *~~~~~~~~~Code~~~~~~~~~~~*
      printf("  br label %%exit%d\n", exitCurrent);

      // Code generation for Exit from if-then-else label
      printf("exit%d:\n", exitCurrent);
   }
   else
   {
      llvmIf(variablesCounter - 1);
      jump1 = jumpCounter - 1;

      // Code Generation for ifBranch label:
      printf("jump%d:\n", jump1);
      // *~~~~~~~~~Code~~~~~~~~~~~*
      parser(ifb, symt, fnList);
      // *~~~~~~~~~Code~~~~~~~~~~~*
      printf("  br label %%exit%d\n", exitCurrent);

      // Code generation for Exit from if-then-else label
      printf("exit%d:\n", exitCurrent);

   }



   // Freeing memory block
   // if (postfx != NULL)
   // {
   //    free(postfx);
   //    postfx = NULL;
   // }
   // if (eTree != NULL)
   // {
   //    forestFire(eTree);
   //    eTree = NULL;
   // }

   return;
}

void whileStatement(char cond[], char whileb[], SymbolTable *symt, functionsList *fnList)
{
   char *postfx = NULL;
   ExpTree *eTree = NULL;
   int jumpHead;
   int jumpBody;
   int exitCurrent = exitCounter;

   if (error)
      return;
   // printf("\n=============SimpleC============\n");
   // printf("condition>\n'%s'\n", condition);
   // printf("whileBody>   \n'%s'\n", whileBody);
   // printf("===================================\n");

   printf("\n  ; while %s\n", cond);
   // Jumping to condition in while
   printf("  br label %%jump%d\n", jumpCounter);
   jumpHead = jumpCounter;
   jumpCounter++;
   jumpBody = jumpCounter;
   jumpCounter++;

   // head:
   printf("jump%d:\n", jumpHead);

   // Generating condition code
   postfx = infixToPostfix(cond, symt, fnList);
   eTree = postfixToTree(postfx, symt);
   SolveTree(eTree);

   // Jump to body or to exit:
   llvmWhile(variablesCounter - 1, jumpBody, exitCurrent);
   exitCounter++;

   // Body:
   printf("jump%d:\n", jumpBody);
   // Generating body code:
   parser(whileb, symt, fnList);

   // Jump to head:
   printf("  br label %%jump%d\n", jumpHead);

   // Jump to exit
   printf("exit%d:\n", exitCurrent);
}

int parser(char *input, SymbolTable *st, functionsList *fnList)
{
   SymbolTable *symbolTable = st;
   SymbolTable *tableTemp = NULL;
   char *expression = NULL;
   ExpTree *tree = NULL;
   char *postfix = NULL;
   char *varName = NULL;
   char *newData = NULL;
   char *condition = NULL;
   char *ifBody = NULL;
   char *elseBody = NULL;
   char *whileBody = NULL;
   int i, tmp, len, counter, index;
   int oldPosition;
   int numLP, numRp;
   int numLC, numRC;
   int jump1, jump2, exit;
   int success = 1;

   if (error)
      return 0;
   // Length of the input
   len = strlen(input);

   for (i = 0; i < len; i++)
   {
      // Skipping unreadable characters
      while ((i < len) && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t'))
         i++;
      if (i >= len)
         break;

      // if Declaration detected:
      if (tmp = checkSequence(input, i, len, "int "))
      {
         i = tmp;
         counter = 0;
         index = 0;

         // Skipping unreadable characters
         while ((i < len) && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t'))
            i++;

         // Getting identifier length
         while (i < len && (isdigit(input[i]) || isalpha(input[i])))
         {
            counter++;
            i++;
         }
         if (counter == 0)
         {
            i = tmp - 4;
            break;
         }

         i = tmp;
         varName = calloc(counter + 1, sizeof(char));

         // Skipping unreadable characters
         while ((i < len) && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t'))
            i++;

         while (i < len && (isdigit(input[i]) || isalpha(input[i])))
            varName[index++] = input[i++];
         varName[index] = '\0';


         while (i < len && input[i] != ';')
            i++;

         // Working with symbol table:
         if (symbolTable == NULL)
         {
            symbolTable = calloc(1, sizeof(SymbolTable));
            symbolTable->name = varName;
            symbolTable->adress = variablesCounter;
            llvmDeclaration(symbolTable->adress);
            variablesCounter++;
         }
         else
         {
            tableTemp = symbolTable;
            // If dublicate found at root
            if (strcmp(tableTemp->name, varName) == 0)
            {
               fprintf(stderr, "error: multiple definitions of %s\n", varName);
               error = 1;
               free(varName);
                  varName = NULL;
               freeSymbolTable(symbolTable);
                  symbolTable = NULL;
               free(input);
                  input = NULL;
               return 0;
            }
            while (tableTemp->next != NULL)
            {
               if (strcmp(tableTemp->name, varName) == 0)
               {
                  fprintf(stderr, "error: multiple definitions of %s\n", varName);
                  error = 1;
                  free(varName);
                     varName = NULL;
                  freeSymbolTable(symbolTable);
                     symbolTable = NULL;
                  free(input);
                     input = NULL;
                  return 0;
               }
               tableTemp = tableTemp->next;
            }
            tableTemp->next = calloc(1, sizeof(SymbolTable));
            tableTemp->next->name = varName;
            tableTemp->next->adress = variablesCounter;
            llvmDeclaration(tableTemp->next->adress);
            variablesCounter++;
         }
      }
      // if PRINT detected:
      else if (tmp = checkSequence(input, i, len, "print "))
      {
         i = tmp;
         counter = 0;
         index = 0;

         // Getting expression length
         while (input[i] != ';' && i < len)
         {
            if (input[i] != ' ' && input[i] != '\n')
               counter++;
            i++;
         }

         // Copying formatted expression in dynamically allocated char array
         i = tmp;
         expression = calloc(counter + 1, sizeof(char));
         while (input[i] != ';' && i < len)
         {
            if (input[i] != ' ' && input[i] != '\n')
               expression[index++] = input[i];
            i++;
         }
         expression[index] = '\0';

         // Evaluating postfix in order to get resulting LLVM instructions
         postfix = infixToPostfix(expression, symbolTable, fnList);

         if (error || postfix == NULL)
         {
            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }

            return 0;
         }

         // Creating expression tree
         tree = postfixToTree(postfix, symbolTable);
         if (error || tree == NULL)
         {
            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }
            if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
            {
               forestFire(tree);
               tree = NULL;
            }

            return 0;
         }

         // getting result at root
         SolveTree(tree);

         if (error)
         {
            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }
            if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
            {
               forestFire(tree);
               tree = NULL;
            }

            return 0;
         }

         if (tree->dataType == 1)
         {
            fprintf(stderr, "error: type-checking: attempting to print Boolean\n");

            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }
            if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
            {
               forestFire(tree);
               tree = NULL;
            }

            return 0;
         }

         if (expressionIsConstant(tree->data))
            llvmPrintConstant(atoi(tree->data));
         else if (tree->llvmVar == 1)
         {
            newData = llvmLoad(tree->data);
            llvmPrintVariable(newData);
         }
         else
            llvmPrintVariable(tree->data);




         // Freeing allocated memory block
         if (newData != NULL)
         {
            free(newData);
            newData = NULL;
         }
         if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
         {
            forestFire(tree);
            tree = NULL;
         }
         if (expression != NULL)
         {
            free(expression);
            expression = NULL;
         }
         if (postfix != NULL)
         {
            free(postfix);
            postfix = NULL;
         }
      }
      // if Read detected:
      else if (tmp = checkSequence(input, i, len, "read "))
      {
         i = tmp;
         counter = 0;
         index = 0;

         while (i < len && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t'))
            i++;

         oldPosition = i;
         while (i < len && input[i] != ' ' && input[i] != ';')
         {
            counter++;
            i++;
         }
         i = oldPosition;
         varName = calloc(counter + 1, sizeof(char));
         while (i < len && input[i] != ' ' && input[i] != ';')
         {
            varName[index++] = input[i];
            i++;
         }
         varName[strlen(varName)] = '\0';
         tmp = symbolTableContains(symbolTable, varName);
         if (!tmp)
         {
            fprintf(stderr, "error: use of undeclared variable %s\n", varName);
            error = 1;
            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (symbolTable != NULL)
            {
               free(symbolTable);
               symbolTable = NULL;
            }
            if (varName != NULL)
            {
               free(varName);
               varName = NULL;
            }

            return 0;
         }
         else
            llvmReadInteger(tmp);

         while (i < len && input[i] != ';')
            i++;
      }
      // if Assignment detected:
      else if (tmp = checkAssignmentSequence(symbolTable, input, i, len))
      {
         counter = 0;
         index = 0;

         while(input[i] != '=')
            i++;
         i++;

         oldPosition = i;

         // Counting expression length
         while(input[i] != ';' && i < len)
         {
            if (input[i] != ' ' && input[i] != '\n')
               counter++;
            i++;
         }

         i = oldPosition;
         expression = calloc(counter + 1, sizeof(char));
         while (input[i] != ';' && i < len)
         {
            if (input[i] != ' ' && input[i] != '\n')
               expression[index++] = input[i];
            i++;
         }
         expression[index] = '\0';

         // Evaluating postfix in order to get resulting LLVM instructions
         postfix = infixToPostfix(expression, symbolTable, fnList);

         if (expression != NULL)
         {
            free(expression);
            expression = NULL;
         }

         if (postfix == NULL || error)
         {
            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }

            return 0;
         }

         // Creating tree from postfix
         tree = postfixToTree(postfix, symbolTable);

         if (tree == NULL || error)
         {
            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }
            if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
            {
               forestFire(tree);
               tree = NULL;
            }

            return 0;
         }
         SolveTree(tree);

         if (error)
         {
            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }
            if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
            {
               forestFire(tree);
               tree = NULL;
            }
            if (postfix != NULL)
            {
               free(postfix);
               postfix = NULL;
            }

            return 0;
         }

         if (tree->dataType == 1)
         {
            fprintf(stderr, "error: type-checking: assignment of boolean into integer\n");

            if (input != NULL)
            {
               free(input);
               input = NULL;
            }
            if (expression != NULL)
            {
               free(expression);
               expression = NULL;
            }
            if (symbolTable != NULL)
            {
               freeSymbolTable(symbolTable);
               symbolTable = NULL;
            }
            if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
            {
               forestFire(tree);
               tree = NULL;
            }

            return 0;
         }

         if (expressionIsConstant(tree->data))
            llvmAssignmentToConstant(tmp, atoi(tree->data));
         else if (tree->llvmVar == 1)
         {
            newData = llvmLoad(tree->data);
            llvmAssignmentToVar(tmp, newData);
         }
         else
            llvmAssignmentToVar(tmp, tree->data);

         // Freeing allocated memory
         if (newData != NULL)
         {
            free(newData);
            newData = NULL;
         }
         if (expression != NULL)
         {
            free(expression);
            expression = NULL;
         }
         if (tree != NULL && sizeof(tree) > sizeof(ExpTree))
         {
            forestFire(tree);
            tree = NULL;
         }
         if (postfix != NULL)
         {
            free(postfix);
            postfix = NULL;
         }
      }
      // if IF statement detected:
      else if (tmp = checkSequence(input, i, len, "if"))
      {
      // Getting condition, ifbody, and elseBody buffered:
      {
         // Condition
         {
            i = tmp;
            counter = 0;
            index = 0;
            numLP = 0;
            numRp = 0;

            while (i < len && input[i] != '(')
               i++;
            oldPosition = i;

            // Getting condition length at counter:
            while (i < len && (numLP == 0 || numLP != numRp))
            {
               if (input[i] == '(')
                  numLP++;
               else if (input[i] == ')')
                  numRp++;

               if (input[i] != ' ')
                  counter++;
               i++;
            }

            i = oldPosition;
            condition = calloc(counter + 1, sizeof(char));
            condition[counter] = '\0';

            while (counter != 0)
            {
               if (input[i] == ' ')
               {
                  i++;
                  continue;
               }

               condition[index] = input[i];
               counter--;

               i++;
               index++;
            }
         }

         // Skipping spaces and unreadable chars
         while (input[i] == ' ' || input[i] == '\n' || input[i] == '\t')
            i++;

         // Getting "if-body" buffered:
         if (input[i] == '{')
         {
            numLC = 0;
            numRC = 0;
            index = 0;
            counter = 0;
            oldPosition = i;

            while (i < len && (numLC == 0 || numLC != numRC))
            {
               if (input[i] == '{')
                  numLC++;
               else if (input[i] == '}')
                  numRC++;

               counter++;
               i++;
            }
            // We do not count Curly braces
            counter -= 2;

            ifBody = calloc(counter + 2, sizeof(char));
            ifBody[counter+1] = '\0';
            ifBody[counter] = ' ';
            i = oldPosition + 1;

            // Copying if body to buffer:
            while (i < len && counter != 0)
            {
               ifBody[index] = input[i];
               counter--;
               index++;
               i++;
            }

            if (input[i] == '}')
               i++;
         }
         else
         {
            index = 0;
            counter = 0;
            oldPosition = i;

            while (i < len)
            {
               counter++;
               if (input[i] == ';')
               {
                  i++;
                  break;
               }
               i++;
            }
            i = oldPosition;

            ifBody = calloc(counter + 2, sizeof(char));
            ifBody[counter] = ' ';
            ifBody[counter+1] = '\0';
            i = oldPosition;

            // Copying if body to buffer:
            while (i < len && counter != 0)
            {
               ifBody[index] = input[i];
               counter--;
               index++;
               i++;
            }
         }

         // Skipping spaces and unreadable chars
         while (input[i] == ' ' || input[i] == '\n' || input[i] == '\t')
            i++;


         if (tmp = checkSequence(input, i, len, "else if"))
         {
            i = tmp - 2;
            numLC = 0;
            numRC = 0;
            index = 0;
            counter = 0;

            oldPosition = i;
            while (i < len && numRC != 2)
            {
               if (input[i] == '}')
                  numRC++;
               i++;
               counter++;
            }
            elseBody = calloc(counter + 2, sizeof(char));
            elseBody[counter+1] = '\0';
            elseBody[counter] = ' ';
            i = oldPosition;

            while (counter != 0)
            {
               elseBody[index] = input[i];
               index++;
               counter--;
               i++;
            }
         }
         // Getting else-body if exists
         else if (tmp = checkSequence(input, i, len, "else"))
         {

            i = tmp;
            // Skipping spaces and unreadable chars
            while (i < len && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t'))
               i++;

            // Getting else-body buffered:
            if (input[i] == '{')
            {
               numLC = 0;
               numRC = 0;
               index = 0;
               counter = 0;
               oldPosition = i;

               while (i < len && (numLC == 0 || numLC != numRC))
               {
                  if (input[i] == '{')
                     numLC++;
                  else if (input[i] == '}')
                     numRC++;

                  counter++;
                  i++;
               }
               // We do not count Curly braces
               counter -= 2;

               elseBody = calloc(counter + 2, sizeof(char));
               elseBody[counter+1] = '\0';
               elseBody[counter] = ' ';
               i = oldPosition + 1;

               // Copying else-body to buffer:
               while (i < len && counter != 0)
               {
                  elseBody[index] = input[i];
                  counter--;
                  index++;
                  i++;
               }

               if (input[i] == '}')
                  i++;
            }
            else
            {
               index = 0;
               counter = 0;
               oldPosition = i;

               while (i < len)
               {
                  counter++;
                  if (input[i] == ';')
                  {
                     i++;
                     break;
                  }
                  i++;
               }
               i = oldPosition;

               elseBody = calloc(counter + 2, sizeof(char));
               elseBody[counter] = ' ';
               elseBody[counter+1] = '\0';
               i = oldPosition;

               // Copying else-body to buffer:
               while (i < len && counter != 0)
               {
                  elseBody[index] = input[i];
                  counter--;
                  index++;
                  i++;
               }
            }

         }
         // If no else-body detected:
         else
         {
            i--;
         }
      }

         // printf("\n=============SimpleC============\n");
         // printf("condition>\n'%s'\n", condition);
         // printf("ifBody>   \n'%s'\n", ifBody);
         // printf("elseBody> \n'%s'\n", elseBody);
         // printf("===================================\n");

         ifThenElseStatement(condition, ifBody, elseBody, symbolTable, fnList);
         i--;
         continue;
      }
      // If while loop detected:
      else if (tmp = checkSequence(input, i, len, "while"))
      {
         // Getting condition and whileBody buffered
         {
            // Condition
            {i = tmp;
            counter = 0;
            index = 0;
            numLP = 0;
            numRp = 0;

            while (i < len && input[i] != '(')
               i++;
            oldPosition = i;

            // Getting condition length at counter:
            while (i < len && (numLP == 0 || numLP != numRp))
            {
               if (input[i] == '(')
                  numLP++;
               else if (input[i] == ')')
                  numRp++;

               if (input[i] != ' ')
                  counter++;
               i++;
            }

            i = oldPosition;
            condition = calloc(counter + 1, sizeof(char));
            condition[counter] = '\0';

            while (counter != 0)
            {
               if (input[i] == ' ')
               {
                  i++;
                  continue;
               }

               condition[index] = input[i];
               counter--;

               i++;
               index++;
            }}
            // Skipping spaces and unreadable chars
            while (i < len && (input[i] == ' ' || input[i] == '\n' || input[i] == '\t'))
               i++;
            // Getting "while-body" buffered:
            if (input[i] == '{')
            {
               numLC = 0;
               numRC = 0;
               index = 0;
               counter = 0;
               oldPosition = i;

               while (i < len && (numLC == 0 || numLC != numRC))
               {
                  if (input[i] == '{')
                     numLC++;
                  else if (input[i] == '}')
                     numRC++;

                  counter++;
                  i++;
               }
               // We do not count Curly braces
               counter -= 2;

               whileBody = calloc(counter + 2, sizeof(char));
               whileBody[counter+1] = '\0';
               whileBody[counter] = ' ';
               i = oldPosition + 1;

               // Copying while body to buffer:
               while (i < len && counter != 0)
               {
                  whileBody[index] = input[i];
                  counter--;
                  index++;
                  i++;
               }

               if (input[i] == '}')
                  i++;
            }
            else
            {
               index = 0;
               counter = 0;
               oldPosition = i;

               while (i < len)
               {
                  counter++;
                  if (input[i] == ';')
                  {
                     i++;
                     break;
                  }
                  i++;
               }
               i = oldPosition;

               whileBody = calloc(counter + 2, sizeof(char));
               whileBody[counter] = ' ';
               whileBody[counter+1] = '\0';
               i = oldPosition;

               // Copying if body to buffer:
               while (i < len && counter != 0)
               {
                  whileBody[index] = input[i];
                  counter--;
                  index++;
                  i++;
               }
            }
         }

         whileStatement(condition, whileBody, symbolTable, fnList);
         i--;
         continue;
      }
      // If return statement detected
      else if (tmp = checkSequence(input, i, len, "return "))
      {
         i = tmp;
         oldPosition = i;
         index = 0;
         counter = 0;

         while (i < len && input[i] != ';')
         {
            counter++;
            i++;
         }

         i = oldPosition;
         expression = calloc(counter + 1, sizeof(char));
         expression[counter] = '\0';

         while (counter != 0)
         {
            expression[index] = input[i];
            index++;
            i++;
            counter--;
         }

         postfix = infixToPostfix(expression, symbolTable, fnList);
         if (expressionIsConstant(postfix))
         {
            printf("  ret i32 %d\n", atoi(postfix));
         }
         else
         {
            tree = postfixToTree(postfix, symbolTable);
            SolveTree(tree);
            if (tree->llvmVar == 1)
            {
               printf("  %%t%d = load i32, i32* %s\n", variablesCounter, tree->data);
               variablesCounter++;
            }

            printf("  ret i32 %%t%d\n", variablesCounter - 1);
         }


      }
      // If Assignment of undeclared variable detected:
      else if (tmp == 0 && i != len - 1 && input[i] > 'a' && input[i] < 'z')
      {
         fprintf(stderr, "error: use of undeclared variable ");
         error = 1;

         while (1)
         {
            fprintf(stderr, "%c", input[i]);
            i++;
            if (input[i] == ' ' || input[i] == '=' || input[i] == ';' || i == len)
               break;
         }

         fprintf(stderr, "\n");

         if (input != NULL)
            free(input);
         if (symbolTable != NULL)
            freeSymbolTable(symbolTable);

         return 0;
      }
   }

   // if (input != NULL)
   // {
   //    free(input);
   //    input = NULL;
   // }
   // if (symbolTable != NULL && sizeof(symbolTable) > sizeof(SymbolTable))
   // {
   //    freeSymbolTable(symbolTable);
   //    symbolTable = NULL;
   // }

   return 1;
}

int main (int args, char *argv[])
{
   functionsList *functions = NULL;
   SymbolTable *localSymbolTable = NULL;
   char *program = NULL;
   char *fnName = NULL;
   char *fnBody = NULL;
   char *parameter = NULL;

   int success = 1;
   int i, len;
   int index, counter;
   int tmp;
   int numLC, numRC;
   int oldPosition;

   program = getInputAsCharArray(argv[1]);
   if (error)
   {
      fprintf(stderr, "fatal error: no input files\n");
      return 0;
   }
   len = strlen(program);

   printf("%s", PROJ4_PROLOGUE);

   for (i = 0; i < len; i++)
   {
      // Skipping non-termials:
      while(i < len && (program[i] == ' ' || program[i] == '\n' || program[i] == '\t'))
         i++;
      if (i >= len)
         break;

      if (tmp = checkSequence(program, i, len, "int "))
      {
         // Getting function name buffered
         {
            counter = 0;
            index = 0;
            i = tmp;

            while (program[i] != '(')
            {
               counter++;
               i++;
            }
            i = tmp;
            fnName = calloc(counter + 1, sizeof(char));
            fnName[counter] = '\0';

            while (counter != 0)
            {
               fnName[index] = program[i];
               index++;
               i++;
               counter--;
            }
         }
         i++;

         // If a function has parameters:
         if (program[i] != ')')
         {
            while (program[i] != ')')
            {
               if (tmp = checkSequence(program, i, len, "int "))
               {
                  i = tmp;
                  counter = 0;
                  index = 0;

                  while (program[i] != ',' && program[i] != ')')
                  {
                     counter++;
                     i++;
                  }
                  parameter = calloc(counter + 1, sizeof(char));
                  parameter[counter] = '\0';
                  i = tmp;

                  while (counter != 0)
                  {
                     parameter[index] = program[i];
                     index++;
                     i++;
                     counter--;
                  }
               }
               while (i < len - 1 && program[i] != 'i' && program[i] != ')')
                  i++;

               // inserting a parameter in fn symbol table
               localSymbolTable = symbolTableAdd(localSymbolTable, parameter, variablesCounter);
               variablesCounter++;
               parameter = NULL;
            }
         }

         // Code generation for function definition:
         llvmDefineFunction(fnName, variablesCounter - 1);

         while (i < len && program[i] != '{')
            i++;
         // Getting function body buffered:
         {
            numLC = 0;
            numRC = 0;
            index = 0;
            counter = 0;
            oldPosition = i;

            while (i < len && (numLC == 0 || numLC != numRC))
            {
               if (program[i] == '{')
                  numLC++;
               else if (program[i] == '}')
                  numRC++;

               counter++;
               i++;
            }
            // We do not count Curly braces
            counter -= 2;

            fnBody = calloc(counter + 2, sizeof(char));
            fnBody[counter+1] = '\0';
            fnBody[counter] = ' ';
            i = oldPosition + 1;

            // Copying while body to buffer:
            while (i < len && counter != 0)
            {
               fnBody[index] = program[i];
               counter--;
               index++;
               i++;
            }
      }

         if (fnListContains(functions, fnName))
         {
            error = 1;
            fprintf(stderr, "error: multiple definitions of %s\n", fnName);
            return 0;
         }

         // Adding function to functionsList
         functions = fnListAdd(functions, fnName, localSymbolTable);

         // Parsing function's Body
         if (parser(fnBody, localSymbolTable, functions) == 0)
            return 0;
         printf("}\n");

         variablesCounter = 1;

         fnBody = NULL;
         localSymbolTable = NULL;

         printf("\n");

         // if (fnBody != NULL)
         //    free(fnBody);
         // if (localSymbolTable != NULL)
         //    freeSymbolTable(localSymbolTable);
         // fnBody = NULL;

      }
   }


   // success = parser(mainFunction, NULL);
   // if (!success)
   //    return 0;


   return 0;
}
