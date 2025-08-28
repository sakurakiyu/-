#include <stdio.h>      // 标准输入输出函数
#include <stdlib.h>     // 内存分配、随机数等函数
#include <time.h>       // 时间相关函数（用于计时）
#include <string.h>     // 字符串处理函数（此处未直接使用，预留）
#include <limits.h>     // 定义各种数据类型的极限值（此处未直接使用，预留）

/********************** 数据结构定义 ************************/

// 多项式链表节点（用于输入存储和临时处理）
typedef struct PolyNode {
    int coef;           // 系数（多项式项的系数）
    int exp;            // 指数（多项式项的指数）
    struct PolyNode *next;  // 指向下一项的指针（构成单链表）
} PolyNode;

// AVL树节点（自平衡二叉搜索树，用于多项式存储与运算）
typedef struct AVLNode {
    int coef;           // 系数
    int exp;            // 指数（作为AVL树的键值，用于排序）
    int height;         // 节点高度（用于维持AVL树平衡）
    struct AVLNode *left, *right;  // 左右子树指针
} AVLNode;

/********************** 工具函数 ************************/

/**
 * 打印多项式中的一项
 * @param coef  项的系数
 * @param exp   项的指数
 * @param isFirst  是否为多项式的第一项（影响符号打印）
 */
void printTerm(int coef, int exp, int isFirst) {
    if (coef == 0) return;  // 系数为0的项不打印
    // 非第一项时，根据系数符号打印"+"
    if (!isFirst) {
        if (coef > 0) printf("+");
    }
    // 根据指数分情况打印
    if (exp == 0) {
        printf("%d", coef);  // 常数项（指数为0）
    } else if (exp == 1) {
        // 一次项（指数为1，省略"^1"）
        if (coef == 1) printf("x");
        else if (coef == -1) printf("-x");
        else printf("%dx", coef);
    } else {
        // 高次项（指数>1）
        if (coef == 1) printf("x^%d", exp);
        else if (coef == -1) printf("-x^%d", exp);
        else printf("%dx^%d", coef, exp);
    }
}

/**
 * 获取当前时间（毫秒级），用于计算运算耗时
 * @return 当前时间（秒转换为毫秒 + 纳秒转换为毫秒）
 */
double getTimeMs() {
    struct timespec ts;  // 用于存储时间的结构体（秒+纳秒）
    clock_gettime(CLOCK_MONOTONIC, &ts);  // 获取单调时间（不受系统时间调整影响）
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;  // 转换为毫秒
}

/**
 * 打印分隔线，用于美化输出格式
 */
void printSeparator() {
    printf("\n========================================================\n");
}

/**
 * 打印操作名称（用于标识当前执行的运算类型）
 * @param op 操作代码（1-加法，2-减法，3-乘法，4-除法）
 */
void printOpName(int op) {
    switch(op) {
        case 1: printf("加法"); break;
        case 2: printf("减法"); break;
        case 3: printf("乘法"); break;
        case 4: printf("除法"); break;
        default: printf("未知操作");
    }
}

/********************** 哈希表实现 ************************/

#define TABLE_SIZE 101  // 哈希表大小（质数，减少冲突）
PolyNode* hashTable[TABLE_SIZE];  // 哈希表数组（每个元素是链表头指针，采用链地址法处理冲突）

/**
 * 哈希函数：根据指数计算哈希表索引
 * @param exp 多项式项的指数
 * @return 哈希表索引（确保非负）
 */
int hashFunc(int exp) {
    return (exp % TABLE_SIZE + TABLE_SIZE) % TABLE_SIZE;  // 处理负数指数的情况
}

/**
 * 向哈希表插入多项式项（自动合并同类项）
 * @param coef 项的系数
 * @param exp 项的指数
 */
void insertHash(int coef, int exp) {
    if (coef == 0) return;  // 系数为0的项无需插入
    int index = hashFunc(exp);  // 计算索引
    PolyNode *p = hashTable[index];  // 遍历对应链表
    while (p) {
        if (p->exp == exp) {  // 找到同类项（指数相同）
            p->coef += coef;  // 合并系数
            return;
        }
        p = p->next;
    }
    // 未找到同类项，创建新节点插入链表头部
    PolyNode *newNode = (PolyNode*)malloc(sizeof(PolyNode));
    newNode->coef = coef;
    newNode->exp = exp;
    newNode->next = hashTable[index];  // 头插法
    hashTable[index] = newNode;
}

/**
 * 清空哈希表，释放所有节点内存
 */
void clearHashTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        PolyNode *p = hashTable[i];
        while (p) {
            PolyNode *temp = p;
            p = p->next;
            free(temp);  // 释放当前节点
        }
        hashTable[i] = NULL;  // 链表头置空
    }
}

/**
 * qsort排序回调函数：按指数降序排列多项式项
 */
int cmpDesc(const void *a, const void *b) {
    return (*(PolyNode**)b)->exp - (*(PolyNode**)a)->exp;
}

/**
 * 打印哈希表中存储的多项式（按指数降序）
 */
void printHash() {
    PolyNode *arr[1000];  // 临时数组存储所有非零项
    int count = 0;
    // 收集哈希表中所有非零系数的项
    for (int i = 0; i < TABLE_SIZE; i++) {
        PolyNode *p = hashTable[i];
        while (p) {
            if (p->coef != 0) arr[count++] = p;
            p = p->next;
        }
    }
    // 按指数降序排序
    qsort(arr, count, sizeof(PolyNode*), cmpDesc);
    // 打印所有项
    for (int i = 0; i < count; i++) {
        printTerm(arr[i]->coef, arr[i]->exp, i == 0);
    }
    if (count == 0) printf("0");  // 空多项式打印0
}

/**
 * 哈希表实现多项式加法：将两个多项式的项插入哈希表（自动合并同类项）
 * @param poly1 第一个多项式（链表）
 * @param poly2 第二个多项式（链表）
 */
void addHash(PolyNode *poly1, PolyNode *poly2) {
    for (PolyNode *p = poly1; p; p = p->next) insertHash(p->coef, p->exp);
    for (PolyNode *p = poly2; p; p = p->next) insertHash(p->coef, p->exp);
}

/**
 * 哈希表实现多项式减法：poly1 - poly2（插入poly1的项和poly2的负项）
 * @param poly1 被减多项式
 * @param poly2 减多项式
 */
void subtractHash(PolyNode *poly1, PolyNode *poly2) {
    for (PolyNode *p = poly1; p; p = p->next) insertHash(p->coef, p->exp);
    for (PolyNode *p = poly2; p; p = p->next) insertHash(-p->coef, p->exp);  // 系数取负
}

/**
 * 哈希表实现多项式乘法：逐项相乘（系数相乘，指数相加）
 * @param poly1 第一个多项式
 * @param poly2 第二个多项式
 */
void multiplyHash(PolyNode *poly1, PolyNode *poly2) {
    for (PolyNode *p1 = poly1; p1; p1 = p1->next) {
        for (PolyNode *p2 = poly2; p2; p2 = p2->next) {
            insertHash(p1->coef * p2->coef, p1->exp + p2->exp);  // 系数积，指数和
        }
    }
}

/**
 * 哈希表实现多项式除法：仅支持除以单项式（poly1 / 单项式）
 * @param poly1 被除多项式
 * @param poly2 除多项式（必须是单项式）
 */
void divideHash(PolyNode *poly1, PolyNode *poly2) {
    if (!poly2 || !poly2->next) {  // 检查是否为单项式（只有一项）
        int c = poly2->coef, e = poly2->exp;
        for (PolyNode *p = poly1; p; p = p->next) {
            insertHash(p->coef / c, p->exp - e);  // 系数商，指数差
        }
    } else {
        printf("提示：哈希表除法仅支持除以单项式\n");
    }
}

/**
 * 计算哈希表占用的总内存空间
 * @return 总空间（字节）：哈希表数组 + 所有节点
 */
size_t calculateHashSpace() {
    size_t space = TABLE_SIZE * sizeof(PolyNode*);  // 哈希表数组本身的空间
    for (int i = 0; i < TABLE_SIZE; i++) {
        PolyNode *p = hashTable[i];
        while (p) {
            space += sizeof(PolyNode);  // 每个节点的空间
            p = p->next;
        }
    }
    return space;
}

/********************** AVL树实现 ************************/

/**
 * 获取AVL树节点的高度（空节点高度为0）
 * @param node 目标节点
 * @return 节点高度
 */
int height(AVLNode *node) { return node ? node->height : 0; }

/**
 * 取两个整数的最大值
 */
int max(int a, int b) { return a > b ? a : b; }

/**
 * 创建新的AVL树节点
 * @param coef 系数
 * @param exp 指数
 * @return 新节点指针
 */
AVLNode* newAVLNode(int coef, int exp) {
    AVLNode *node = (AVLNode*)malloc(sizeof(AVLNode));
    node->coef = coef;
    node->exp = exp;
    node->height = 1;  // 新节点高度为1
    node->left = node->right = NULL;  // 左右子树为空
    return node;
}

/**
 * AVL树右旋转操作（用于平衡调整）
 * @param y 失衡节点
 * @return 旋转后新的根节点（x）
 */
AVLNode* rightRotate(AVLNode *y) {
    AVLNode *x = y->left;       // x是y的左子节点
    AVLNode *T2 = x->right;     // T2是x的右子树
    x->right = y;               // y成为x的右子节点
    y->left = T2;               // T2成为y的左子树
    // 更新高度
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;  // x成为新的根节点
}

/**
 * AVL树左旋转操作（用于平衡调整）
 * @param x 失衡节点
 * @return 旋转后新的根节点（y）
 */
AVLNode* leftRotate(AVLNode *x) {
    AVLNode *y = x->right;      // y是x的右子节点
    AVLNode *T2 = y->left;      // T2是y的左子树
    y->left = x;                // x成为y的左子节点
    x->right = T2;              // T2成为x的右子树
    // 更新高度
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;  // y成为新的根节点
}

/**
 * 计算AVL节点的平衡因子（左子树高度 - 右子树高度）
 * @param node 目标节点
 * @return 平衡因子（范围[-1,1]为平衡）
 */
int getBalance(AVLNode *node) { return node ? height(node->left) - height(node->right) : 0; }

/**
 * 向AVL树插入节点（自动平衡）
 * @param node 当前节点（递归参数）
 * @param coef 插入项的系数
 * @param exp 插入项的指数（作为键值）
 * @return 插入后平衡的子树根节点
 */
AVLNode* insertAVL(AVLNode *node, int coef, int exp) {
    // 1. 执行标准BST插入
    if (!node) return newAVLNode(coef, exp);  // 空树，创建新节点
    if (exp < node->exp)
        node->left = insertAVL(node->left, coef, exp);  // 指数小的放左子树
    else if (exp > node->exp)
        node->right = insertAVL(node->right, coef, exp);  // 指数大的放右子树
    else {
        node->coef += coef;  // 指数相同，合并系数
        return node;
    }
    // 2. 更新当前节点高度
    node->height = 1 + max(height(node->left), height(node->right));
    // 3. 计算平衡因子，检查是否失衡
    int balance = getBalance(node);
    // 4. 处理四种失衡情况，进行旋转调整
    // 左左失衡（左子树的左子树过高）
    if (balance > 1 && exp < node->left->exp)
        return rightRotate(node);
    // 右右失衡（右子树的右子树过高）
    if (balance < -1 && exp > node->right->exp)
        return leftRotate(node);
    // 左右失衡（左子树的右子树过高）
    if (balance > 1 && exp > node->left->exp) {
        node->left = leftRotate(node->left);  // 先左旋转左子树
        return rightRotate(node);  // 再右旋转当前节点
    }
    // 右左失衡（右子树的左子树过高）
    if (balance < -1 && exp < node->right->exp) {
        node->right = rightRotate(node->right);  // 先右旋转右子树
        return leftRotate(node);  // 再左旋转当前节点
    }
    return node;  // 未失衡，返回原节点
}

/**
 * 前序遍历AVL树，并将节点插入到目标AVL树中
 * @param result 目标AVL树（指针的指针，用于修改根节点）
 * @param node 源AVL树的当前节点
 */
void preOrderInsert(AVLNode **result, AVLNode *node) {
    if (!node) return;
    *result = insertAVL(*result, node->coef, node->exp);  // 插入当前节点
    preOrderInsert(result, node->left);  // 递归左子树
    preOrderInsert(result, node->right);  // 递归右子树
}

/**
 * AVL树实现多项式加法：合并两个AVL树的节点
 * @param poly1 第一个多项式（AVL树）
 * @param poly2 第二个多项式（AVL树）
 * @return 结果多项式（AVL树）
 */
AVLNode* addAVL(AVLNode *poly1, AVLNode *poly2) {
    AVLNode *result = NULL;
    preOrderInsert(&result, poly1);  // 插入第一个多项式的所有节点
    preOrderInsert(&result, poly2);  // 插入第二个多项式的所有节点（自动合并同类项）
    return result;
}

/**
 * AVL树实现多项式减法：poly1 - poly2（插入poly1的节点和poly2的负节点）
 * @param poly1 被减多项式
 * @param poly2 减多项式
 * @return 结果多项式
 */
AVLNode* subtractAVL(AVLNode *poly1, AVLNode *poly2) {
    AVLNode *result = NULL;
    preOrderInsert(&result, poly1);  // 插入poly1的所有节点
    if (!poly2) return result;
    // 插入poly2的节点（系数取负）
    result = insertAVL(result, -poly2->coef, poly2->exp);
    // 递归处理左右子树
    result = subtractAVL(result, poly2->left);
    result = subtractAVL(result, poly2->right);
    return result;
}

/**
 * 辅助函数：将单个AVL节点与另一个AVL树的所有节点相乘，结果插入目标树
 * @param result 目标结果树
 * @param node 单个节点（来自第一个多项式）
 * @param poly 第二个多项式（AVL树）
 */
void multiplyNodeWithAVL(AVLNode **result, AVLNode *node, AVLNode *poly) {
    if (!node || !poly) return;
    multiplyNodeWithAVL(result, node, poly->left);  // 递归左子树
    // 相乘后插入结果树（系数积，指数和）
    *result = insertAVL(*result, node->coef * poly->coef, node->exp + poly->exp);
    multiplyNodeWithAVL(result, node, poly->right);  // 递归右子树
}

/**
 * 辅助函数：递归实现两个AVL树的乘法
 * @param result 目标结果树
 * @param poly1 第一个多项式
 * @param poly2 第二个多项式
 */
void multiplyAVLHelper(AVLNode **result, AVLNode *poly1, AVLNode *poly2) {
    if (!poly1 || !poly2) return;
    multiplyAVLHelper(result, poly1->left, poly2);  // 递归处理poly1的左子树
    multiplyNodeWithAVL(result, poly1, poly2);  // 当前节点与poly2的所有节点相乘
    multiplyAVLHelper(result, poly1->right, poly2);  // 递归处理poly1的右子树
}

/**
 * AVL树实现多项式乘法
 * @param poly1 第一个多项式
 * @param poly2 第二个多项式
 * @return 结果多项式
 */
AVLNode* multiplyAVL(AVLNode *poly1, AVLNode *poly2) {
    if (!poly1 || !poly2) return NULL;  // 空多项式返回空
    AVLNode *result = NULL;  
    multiplyAVLHelper(&result, poly1, poly2);
    return result;
}

/**
 * AVL树实现多项式除法：仅支持除以单项式
 * @param poly1 被除多项式
 * @param poly2 除多项式（必须是单项式）
 * @return 结果多项式
 */
AVLNode* divideAVL(AVLNode *poly1, AVLNode *poly2) {
    // 检查除多项式是否为单项式（无左右子树）
    if (!poly2 || poly2->left || poly2->right) {
        printf("提示：AVL树除法仅支持除以单项式\n");
        return NULL;
    }
    int c = poly2->coef, e = poly2->exp;  // 单项式的系数和指数
    AVLNode *result = NULL;

    if (!poly1) return NULL;
    // 递归处理左右子树
    AVLNode *left = divideAVL(poly1->left, poly2);
    AVLNode *right = divideAVL(poly1->right, poly2);
    // 处理当前节点（系数商，指数差）
    if (poly1->coef != 0) {
        result = insertAVL(result, poly1->coef / c, poly1->exp - e);
    }
    // 合并左右子树的结果
    preOrderInsert(&result, left);
    preOrderInsert(&result, right);
    return result;
}

/**
 * 计算AVL树占用的总内存空间
 * @param root AVL树根节点
 * @return 总空间（字节）：所有节点的大小之和
 */
size_t calculateAVLSpace(AVLNode *root) {
    if (!root) return 0;
    // 递归计算：当前节点大小 + 左子树大小 + 右子树大小
    return sizeof(AVLNode) + calculateAVLSpace(root->left) + calculateAVLSpace(root->right);
}

/**
 * 释放AVL树的所有节点内存（后序遍历）
 * @param node 根节点
 */
void freeAVL(AVLNode *node) {
    if (!node) return;
    freeAVL(node->left);   // 释放左子树
    freeAVL(node->right);  // 释放右子树
    free(node);            // 释放当前节点
}

/**
 * 辅助函数：中序遍历（右-根-左）打印AVL树，确保按指数降序输出
 * @param root 当前节点
 * @param isFirst 标识是否为第一项（指针传递，用于跨递归修改）
 */
void printAVLHelper(AVLNode *root, int *isFirst) {
    if (!root) return;
    printAVLHelper(root->right, isFirst);  // 先打印右子树（指数更大）
    printTerm(root->coef, root->exp, *isFirst);  // 打印当前节点
    *isFirst = 0;  // 后续项不再是第一项
    printAVLHelper(root->left, isFirst);   // 再打印左子树（指数更小）
}

/**
 * 打印AVL树中存储的多项式
 * @param root AVL树根节点
 */
void printAVL(AVLNode *root) {
    int isFirst = 1;  // 初始化为第一项
    printAVLHelper(root, &isFirst);
    if (isFirst) printf("0");  // 空多项式打印0
}

/********************** 输入/转换函数 ************************/

/**
 * 打印输入的多项式（链表形式），按指数降序排列
 * @param poly 多项式链表头
 */
void printInputPoly(PolyNode *poly) {
    if (!poly) {
        printf("0");
        return;
    }
    int isFirst = 1;
    PolyNode *arr[100];  // 临时数组存储所有项
    int count = 0;
    while (poly) {
        arr[count++] = poly;
        poly = poly->next;
    }
    // 冒泡排序：按指数降序
    for (int i = 0; i < count-1; i++) {
        for (int j = i+1; j < count; j++) {
            if (arr[i]->exp < arr[j]->exp) {
                PolyNode *temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
    // 打印排序后的项
    for (int i = 0; i < count; i++) {
        printTerm(arr[i]->coef, arr[i]->exp, isFirst);
        isFirst = 0;
    }
}

/**
 * 手动输入多项式（链表形式）
 * @param termCount 输出参数，存储项的数量
 * @return 多项式链表头
 */
PolyNode* inputPoly(int *termCount) {
    int n;
    printf("请输入多项式项数: ");
    scanf("%d", &n);
    *termCount = n;
    PolyNode *head = NULL;
    for (int i = 0; i < n; i++) {
        int c, e;
        printf("请输入第%d项 (系数 指数): ", i + 1);
        scanf("%d%d", &c, &e);
        // 创建节点，头插法插入链表
        PolyNode *newNode = (PolyNode*)malloc(sizeof(PolyNode));
        newNode->coef = c;
        newNode->exp = e;
        newNode->next = head;
        head = newNode;
    }
    // 显示输入的多项式（排序后）
    printf("输入的多项式: ");
    printInputPoly(head);
    printf("\n");
    return head;
}

/**
 * 随机生成多项式（链表形式）
 * @param termCount 项数
 * @param maxCoef 系数最大值（绝对值）
 * @param maxExp 指数最大值
 * @return 多项式链表头
 */
PolyNode* generatePoly(int termCount, int maxCoef, int maxExp) {
    PolyNode *head = NULL;
    for (int i = 0; i < termCount; i++) {
        PolyNode *node = (PolyNode*)malloc(sizeof(PolyNode));
        // 随机系数：1~maxCoef，50%概率为负
        node->coef = (rand() % maxCoef) + 1;
        if (rand() % 2 == 0) {
            node->coef = -node->coef;
        }
        // 随机指数：0~maxExp
        node->exp = rand() % (maxExp + 1);
        // 头插法插入链表
        node->next = head;
        head = node;
    }
    return head;
}

/**
 * 将多项式链表转换为AVL树
 * @param list 多项式链表头
 * @return AVL树根节点
 */
AVLNode* listToAVL(PolyNode *list) {
    AVLNode *root = NULL;
    while (list) {
        root = insertAVL(root, list->coef, list->exp);  // 逐项插入AVL树
        list = list->next;
    }
    return root;
}

/********************** 主函数 ************************/

int main() {
    while (1) {  // 主循环：选择模式
        printf("\n==== 多项式运算系统 ====\n");
        printf("1. 交互模式\n");
        printf("2. 批量测试模式\n");
        printf("0. 退出\n");
        printf("请选择模式: ");
        int mode;
        scanf("%d", &mode);

        if (mode == 0) {
            printf("程序已退出\n");
            break;
        }

        if (mode == 1) {
            // ========= 交互模式：手动输入多项式并运算 =========
            while (1) {
                int n1, n2;  // 两个多项式的项数
                printSeparator();
                
                // 输入第一个多项式
                printf("第一个多项式：\n");
                PolyNode *poly1 = inputPoly(&n1);
                
                // 输入第二个多项式
                printf("\n第二个多项式：\n");
                PolyNode *poly2 = inputPoly(&n2);
                
                // 选择运算（若第二个是单项式，自动选择除法）
                printSeparator();
                int isDivision = (n2 == 1);  // 单项式判断
                int op = 0;
                if (!isDivision) {
                    printf("请选择运算：1.加法 2.减法 3.乘法\n");
                    scanf("%d", &op);
                } else {
                    printf("检测到第二个多项式是单项式，自动选择除法运算\n");
                    op = 4;  // 除法
                }
                printSeparator();
                
                // 显示当前运算
                printf("当前运算： ");
                printInputPoly(poly1);
                printf(" ");
                switch(op) {
                    case 1: printf("+"); break;
                    case 2: printf("-"); break;
                    case 3: printf("×"); break;
                    case 4: printf("÷"); break;
                }
                printf(" ");
                printInputPoly(poly2);
                printf("\n");
                printSeparator();
                
                // 哈希表运算（计时+计算结果）
                for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;  // 初始化哈希表
                double hash_start = getTimeMs();  // 开始计时
                if (op == 1) addHash(poly1, poly2);
                else if (op == 2) subtractHash(poly1, poly2);
                else if (op == 3) multiplyHash(poly1, poly2);
                else divideHash(poly1, poly2);
                double hash_end = getTimeMs();  // 结束计时
                double hashTime = hash_end - hash_start;  // 耗时
                size_t hashSpace = calculateHashSpace();  // 空间占用
                
                // AVL树运算（计时+计算结果）
                AVLNode *poly1_avl = listToAVL(poly1);  // 转换为AVL树
                AVLNode *poly2_avl = listToAVL(poly2);
                double avl_start = getTimeMs();  // 开始计时
                AVLNode *result_avl = NULL;
                if (op == 1) result_avl = addAVL(poly1_avl, poly2_avl);
                else if (op == 2) result_avl = subtractAVL(poly1_avl, poly2_avl);
                else if (op == 3) result_avl = multiplyAVL(poly1_avl, poly2_avl);
                else result_avl = divideAVL(poly1_avl, poly2_avl);
                double avl_end = getTimeMs();  // 结束计时
                double avlTime = avl_end - avl_start;  // 耗时
                size_t avlSpace = calculateAVLSpace(result_avl);  // 空间占用
                
                // 输出性能对比结果
                printf("===== 运算结果对比 =====\n");
                printf("| 数据结构 | 运算结果 | 耗时 (ms) | 内存占用 (B) |\n");
                printf("|----------|----------|-----------|--------------|\n");
                
                // 哈希表结果
                printf("| 哈希表   | ");
                printHash();
                printf(" | %10.6f | %12zu |\n", hashTime, hashSpace);
                
                // AVL树结果
                printf("| AVL树    | ");
                printAVL(result_avl);
                printf(" | %10.6f | %12zu |\n", avlTime, avlSpace);
                printSeparator();
                
                // 释放内存（防止泄漏）
                while (poly1) { PolyNode *t = poly1; poly1 = poly1->next; free(t); }
                while (poly2) { PolyNode *t = poly2; poly2 = poly2->next; free(t); }
                freeAVL(poly1_avl); 
                freeAVL(poly2_avl); 
                freeAVL(result_avl);
                clearHashTable();
                
                // 选择继续或退出交互模式
                int cont;
                printf("是否继续交互模式？输入 1 继续，输入 0 返回主菜单: ");
                scanf("%d", &cont);
                if (cont == 0) break;
            }
        }

        if (mode == 2) {
            // ========= 批量测试模式：自动生成多项式，多次运算并统计性能 =========
            int termCount1, termCount2, maxCoef, maxExp, testRuns;
            
            // 获取测试参数
            printf("请输入批量测试参数：\n");
            printf("第一个多项式项数: ");
            scanf("%d", &termCount1);
            printf("第二个多项式项数: ");
            scanf("%d", &termCount2);
            printf("最大系数值: ");
            scanf("%d", &maxCoef);
            printf("最大指数值: ");
            scanf("%d", &maxExp);
            printf("测试次数: ");
            scanf("%d", &testRuns);
            
            // 初始化随机数种子
            srand(time(NULL));
            
            // 存储每次测试的耗时
            double hashAddTimes[testRuns], avlAddTimes[testRuns];
            double hashSubTimes[testRuns], avlSubTimes[testRuns];
            double hashMulTimes[testRuns], avlMulTimes[testRuns];
            double hashDivTimes[testRuns], avlDivTimes[testRuns];
            
            // 总内存占用（用于计算平均值）
            size_t totalHashSpace = 0, totalAVLSpace = 0;
            
            printf("\n开始批量测试，共 %d 次...\n", testRuns);
            
            for (int run = 0; run < testRuns; run++) {
                // 自动生成两个多项式（链表）
                PolyNode *poly1 = generatePoly(termCount1, maxCoef, maxExp);
                PolyNode *poly2 = generatePoly(termCount2, maxCoef, maxExp);
                
                // 转换为AVL树
                AVLNode *poly1_avl = listToAVL(poly1);
                AVLNode *poly2_avl = listToAVL(poly2);
                
                // 生成除法用的单项式
                PolyNode *mono = generatePoly(1, maxCoef, maxExp);  // 单项式（1项）
                AVLNode *mono_avl = listToAVL(mono);
                
                // ====== 测试加法 ======
                for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;
                double h_start = getTimeMs(); 
                addHash(poly1, poly2); 
                double h_end = getTimeMs();
                hashAddTimes[run] = h_end - h_start;
                totalHashSpace += calculateHashSpace();
                clearHashTable();  // 清空哈希表
                
                double a_start = getTimeMs(); 
                AVLNode *r1 = addAVL(poly1_avl, poly2_avl); 
                double a_end = getTimeMs();
                avlAddTimes[run] = a_end - a_start;
                totalAVLSpace += calculateAVLSpace(r1);
                freeAVL(r1);  // 释放结果树
                
                // ====== 测试减法 ======
                for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;
                h_start = getTimeMs(); 
                subtractHash(poly1, poly2); 
                h_end = getTimeMs();
                hashSubTimes[run] = h_end - h_start;
                totalHashSpace += calculateHashSpace();
                clearHashTable();
                
                a_start = getTimeMs(); 
                AVLNode *r2 = subtractAVL(poly1_avl, poly2_avl); 
                a_end = getTimeMs();
                avlSubTimes[run] = a_end - a_start;
                totalAVLSpace += calculateAVLSpace(r2);
                freeAVL(r2);
                
                // ====== 测试乘法 ======
                for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;
                h_start = getTimeMs(); 
                multiplyHash(poly1, poly2); 
                h_end = getTimeMs();
                hashMulTimes[run] = h_end - h_start;
                totalHashSpace += calculateHashSpace();
                clearHashTable();
                
                a_start = getTimeMs(); 
                AVLNode *r3 = multiplyAVL(poly1_avl, poly2_avl); 
                a_end = getTimeMs();
                avlMulTimes[run] = a_end - a_start;
                totalAVLSpace += calculateAVLSpace(r3);
                freeAVL(r3);
                
                // ====== 测试除法（仅除以单项式） ======
                for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;
                h_start = getTimeMs(); 
                divideHash(poly1, mono); 
                h_end = getTimeMs();
                hashDivTimes[run] = h_end - h_start;
                totalHashSpace += calculateHashSpace();
                clearHashTable();
                
                a_start = getTimeMs(); 
                AVLNode *r4 = divideAVL(poly1_avl, mono_avl); 
                a_end = getTimeMs();
                avlDivTimes[run] = a_end - a_start;
                if (r4) totalAVLSpace += calculateAVLSpace(r4);
                freeAVL(r4);
                
                // 释放本次测试的内存
                while (poly1) { PolyNode *t = poly1; poly1 = poly1->next; free(t); }
                while (poly2) { PolyNode *t = poly2; poly2 = poly2->next; free(t); }
                while (mono) { PolyNode *t = mono; mono = mono->next; free(t); }
                freeAVL(poly1_avl); 
                freeAVL(poly2_avl); 
                freeAVL(mono_avl);
                
                // 显示进度
                printf("完成测试 %d/%d\n", run + 1, testRuns);
            }
            
            // 计算平均耗时
            double hashAddAvg = 0, avlAddAvg = 0;
            double hashSubAvg = 0, avlSubAvg = 0;
            double hashMulAvg = 0, avlMulAvg = 0;
            double hashDivAvg = 0, avlDivAvg = 0;
            
            for (int i = 0; i < testRuns; i++) {
                hashAddAvg += hashAddTimes[i];
                avlAddAvg += avlAddTimes[i];
                hashSubAvg += hashSubTimes[i];
                avlSubAvg += avlSubTimes[i];
                hashMulAvg += hashMulTimes[i];
                avlMulAvg += avlMulTimes[i];
                hashDivAvg += hashDivTimes[i];
                avlDivAvg += avlDivTimes[i];
            }
            
            hashAddAvg /= testRuns;
            avlAddAvg /= testRuns;
            hashSubAvg /= testRuns;
            avlSubAvg /= testRuns;
            hashMulAvg /= testRuns;
            avlMulAvg /= testRuns;
            hashDivAvg /= testRuns;
            avlDivAvg /= testRuns;
            
            // 输出批量测试结果
            printSeparator();
            printf("===== 批量测试结果（平均值） =====\n");
            printf("测试参数：\n");
            printf("  - 多项式1项数: %d\n", termCount1);
            printf("  - 多项式2项数: %d\n", termCount2);
            printf("  - 最大系数值:  %d\n", maxCoef);
            printf("  - 最大指数值:  %d\n", maxExp);
            printf("  - 测试次数:    %d\n", testRuns);
            printSeparator();
            
            // 加法结果
            printf("--- 加法运算 ---\n");
            printf("| 数据结构 | 平均耗时 (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| 哈希表   | %13.6f |\n", hashAddAvg);
            printf("| AVL树    | %13.6f |\n", avlAddAvg);
            printSeparator();
            
            // 减法结果
            printf("--- 减法运算 ---\n");
            printf("| 数据结构 | 平均耗时 (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| 哈希表   | %13.6f |\n", hashSubAvg);
            printf("| AVL树    | %13.6f |\n", avlSubAvg);
            printSeparator();
            
            // 乘法结果
            printf("--- 乘法运算 ---\n");
            printf("| 数据结构 | 平均耗时 (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| 哈希表   | %13.6f |\n", hashMulAvg);
            printf("| AVL树    | %13.6f |\n", avlMulAvg);
            printSeparator();
            
            // 除法结果
            printf("--- 除法运算 (poly1 ÷ 单项式) ---\n");
            printf("| 数据结构 | 平均耗时 (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| 哈希表   | %13.6f |\n", hashDivAvg);
            printf("| AVL树    | %13.6f |\n", avlDivAvg);
            printSeparator();
            
            // 内存占用结果（总空间 / 测试次数 / 4种运算）
            printf("--- 内存占用（平均值） ---\n");
            printf("| 数据结构 | 平均内存占用 (B) |\n");
            printf("|----------|------------------|\n");
            printf("| 哈希表   | %16.2f |\n", (double)totalHashSpace / (testRuns * 4));
            printf("| AVL树    | %16.2f |\n", (double)totalAVLSpace / (testRuns * 4));
            printSeparator();
        }
    }
    return 0;
}
