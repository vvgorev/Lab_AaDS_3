#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

typedef enum { NUMBER, VARIABLE, OPERATOR } NodeType;

typedef struct Node {
    NodeType type;
    union {
        int num;
        char var[32];
        char op;
    };
    struct Node *left;
    struct Node *right;
} Node;

Node* create_number(int n) {
    Node* node = malloc(sizeof(Node));
    node->type = NUMBER;
    node->num = n;
    node->left = node->right = NULL;
    return node;
}

Node* create_variable(const char* name) {
    Node* node = malloc(sizeof(Node));
    node->type = VARIABLE;
    strcpy(node->var, name);
    node->left = node->right = NULL;
    return node;
}

Node* create_operator(char op, Node* left, Node* right) {
    Node* node = malloc(sizeof(Node));
    node->type = OPERATOR;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

void print_tree(Node* node) {
    if (!node) return;
    if (node->type == NUMBER) {
        printf("%d", node->num);
    } else if (node->type == VARIABLE) {
        printf("%s", node->var);
    } else if (node->type == OPERATOR) {
        printf("(");
        print_tree(node->left);
        printf(" %c ", node->op);
        print_tree(node->right);
        printf(")");
    }
}

Node* simplify_division(Node* node) {
    if (!node) return NULL;

    node->left = simplify_division(node->left);
    node->right = simplify_division(node->right);

    if (node->type == OPERATOR && node->op == '/') {
        Node* left = node->left;
        Node* right = node->right;

        if (left->type == NUMBER && right->type == NUMBER) {
            if (right->num != 0 && left->num % right->num == 0) {
                int result = left->num / right->num;
                free(node); free(left); free(right);
                return create_number(result);
            }
        }

        if (left->type == OPERATOR && left->op == '*' &&
            left->left->type == NUMBER && right->type == NUMBER) {
            int coeff = left->left->num;
            int divisor = right->num;
            if (divisor != 0 && coeff % divisor == 0) {
                int new_coeff = coeff / divisor;
                Node* new_left = create_number(new_coeff);
                Node* new_right = left->right;
                free(left->left); free(left); free(right); free(node);
                return create_operator('*', new_left, new_right);
            }
        }
    }

    return node;
}

Node* parse_simple_expr(const char* expr) {
    int i = 0;
    Node* root = NULL;
    char last_op = 0;

    while (expr[i] != '\0') {
        while (expr[i] == ' ') i++;

        Node* operand = NULL;
        if (isdigit(expr[i]) || expr[i] == '-') {
            int start = i;
            if (expr[i] == '-') i++;
            while (isdigit(expr[i])) i++;
            int len = i - start;
            char buf[32];
            strncpy(buf, expr + start, len);
            buf[len] = '\0';
            operand = create_number(atoi(buf));
        } else if (isalpha(expr[i])) {
            int start = i;
            while (isalnum(expr[i])) i++;
            int len = i - start;
            char buf[32];
            strncpy(buf, expr + start, len);
            buf[len] = '\0';
            operand = create_variable(buf);
        } else {
            break;
        }

        if (!operand) break;

        if (!root) {
            root = operand;
        } else {
            Node* new_root = create_operator(last_op, root, operand);
            root = new_root;
        }

        while (expr[i] == ' ') i++;
        if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
            last_op = expr[i];
            i++;
        } else {
            break;
        }
    }

    return root;
}

int main() {
    SetConsoleOutputCP(1251);
    const char* input = "4 * a / 2";
    printf("Исходное выражение: %s\n", input);

    Node* tree = parse_simple_expr(input);
    if (!tree) {
        printf("Ошибка парсинга!\n");
        return 1;
    }

    printf("Дерево до упрощения: ");
    print_tree(tree);
    printf("\n");

    tree = simplify_division(tree);

    printf("Дерево после упрощения: ");
    print_tree(tree);
    printf("\n");

    return 0;
}