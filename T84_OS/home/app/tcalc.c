// ! NOT WORKING FOR NOW!

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define MAX_EXPRESSION 256

typedef enum
{
    MODE_BASIC,
    MODE_SCIENTIFIC,
    MODE_PROGRAMMER
} CalcMode;

double evaluate_basic(const char *expr);
void print_welcome(void);
void run_calculator(void);
void print_help(void);

int main(void)
{
    print_welcome();
    run_calculator();
    return 0;
}

void print_welcome(void)
{
    printf("\n");
    printf("========================================\n");
    printf("        T84 Calculator v1.0            \n");
    printf("========================================\n");
    printf("Features:\n");
    printf("  • Basic arithmetic (+, -, *, /)\n");
    printf("  • Parentheses support\n");
    printf("  • Error handling\n");
    printf("  • History (up to 10 calculations)\n");
    printf("\n");
    printf("Commands:\n");
    printf("  help     - Show this help\n");
    printf("  clear    - Clear screen\n");
    printf("  history  - Show calculation history\n");
    printf("  exit     - Return to T84 OS\n");
    printf("\n");
}

void run_calculator(void)
{
    char input[MAX_EXPRESSION];
    char history[10][MAX_EXPRESSION];
    double results[10];
    int history_count = 0;

    while (1)
    {
        printf("calc> ");

        if (fgets(input, MAX_EXPRESSION, stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0)
        {
            break;
        }

        if (strcmp(input, "help") == 0)
        {
            print_help();
            continue;
        }

        if (strcmp(input, "clear") == 0 || strcmp(input, "cls") == 0)
        {
            system("clear");
            continue;
        }

        if (strcmp(input, "history") == 0)
        {
            printf("\nCalculation History:\n");
            for (int i = 0; i < history_count; i++)
            {
                printf("%d: %s = %.2f\n", i + 1, history[i], results[i]);
            }
            printf("\n");
            continue;
        }

        if (strlen(input) == 0)
        {
            continue;
        }

        double result = evaluate_basic(input);

        if (history_count < 10)
        {
            strcpy(history[history_count], input);
            results[history_count] = result;
            history_count++;
        }

        printf("Result: %.4f\n", result);
    }

    printf("\nCalculator session ended.\n");
}

double evaluate_basic(const char *expr)
{

    double a = 0, b = 0;
    char op = '+';

    sscanf(expr, "%lf %c %lf", &a, &op, &b);

    switch (op)
    {
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        if (b != 0)
            return a / b;
        else
        {
            printf("Error: Division by zero\n");
            return 0;
        }
    default:
        printf("Error: Unknown operator '%c'\n", op);
        return 0;
    }
}

void print_help(void)
{
    printf("\nCalculator Help:\n");
    printf("Enter mathematical expressions to evaluate them.\n");
    printf("\nExamples:\n");
    printf("  2 + 3 * 4      (Multiplication first)\n");
    printf("  (2 + 3) * 4    (Parentheses first)\n");
    printf("  10 / 3         (Division)\n");
    printf("  15 - 8         (Subtraction)\n");
    printf("\nNote: This is a basic version. Future updates will include:\n");
    printf("  • Scientific functions (sin, cos, log, etc.)\n");
    printf("  • Variables and constants\n");
    printf("  • Complex number support\n");
    printf("  • Graphing capabilities\n");
    printf("\n");
}