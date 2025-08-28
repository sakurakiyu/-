#include <stdio.h>      // ��׼�����������
#include <stdlib.h>     // �ڴ���䡢������Ⱥ���
#include <time.h>       // ʱ����غ��������ڼ�ʱ��
#include <string.h>     // �ַ������������˴�δֱ��ʹ�ã�Ԥ����
#include <limits.h>     // ��������������͵ļ���ֵ���˴�δֱ��ʹ�ã�Ԥ����

/********************** ���ݽṹ���� ************************/

// ����ʽ����ڵ㣨��������洢����ʱ����
typedef struct PolyNode {
    int coef;           // ϵ��������ʽ���ϵ����
    int exp;            // ָ��������ʽ���ָ����
    struct PolyNode *next;  // ָ����һ���ָ�루���ɵ�����
} PolyNode;

// AVL���ڵ㣨��ƽ����������������ڶ���ʽ�洢�����㣩
typedef struct AVLNode {
    int coef;           // ϵ��
    int exp;            // ָ������ΪAVL���ļ�ֵ����������
    int height;         // �ڵ�߶ȣ�����ά��AVL��ƽ�⣩
    struct AVLNode *left, *right;  // ��������ָ��
} AVLNode;

/********************** ���ߺ��� ************************/

/**
 * ��ӡ����ʽ�е�һ��
 * @param coef  ���ϵ��
 * @param exp   ���ָ��
 * @param isFirst  �Ƿ�Ϊ����ʽ�ĵ�һ�Ӱ����Ŵ�ӡ��
 */
void printTerm(int coef, int exp, int isFirst) {
    if (coef == 0) return;  // ϵ��Ϊ0�����ӡ
    // �ǵ�һ��ʱ������ϵ�����Ŵ�ӡ"+"
    if (!isFirst) {
        if (coef > 0) printf("+");
    }
    // ����ָ���������ӡ
    if (exp == 0) {
        printf("%d", coef);  // �����ָ��Ϊ0��
    } else if (exp == 1) {
        // һ���ָ��Ϊ1��ʡ��"^1"��
        if (coef == 1) printf("x");
        else if (coef == -1) printf("-x");
        else printf("%dx", coef);
    } else {
        // �ߴ��ָ��>1��
        if (coef == 1) printf("x^%d", exp);
        else if (coef == -1) printf("-x^%d", exp);
        else printf("%dx^%d", coef, exp);
    }
}

/**
 * ��ȡ��ǰʱ�䣨���뼶�������ڼ��������ʱ
 * @return ��ǰʱ�䣨��ת��Ϊ���� + ����ת��Ϊ���룩
 */
double getTimeMs() {
    struct timespec ts;  // ���ڴ洢ʱ��Ľṹ�壨��+���룩
    clock_gettime(CLOCK_MONOTONIC, &ts);  // ��ȡ����ʱ�䣨����ϵͳʱ�����Ӱ�죩
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;  // ת��Ϊ����
}

/**
 * ��ӡ�ָ��ߣ��������������ʽ
 */
void printSeparator() {
    printf("\n========================================================\n");
}

/**
 * ��ӡ�������ƣ����ڱ�ʶ��ǰִ�е��������ͣ�
 * @param op �������루1-�ӷ���2-������3-�˷���4-������
 */
void printOpName(int op) {
    switch(op) {
        case 1: printf("�ӷ�"); break;
        case 2: printf("����"); break;
        case 3: printf("�˷�"); break;
        case 4: printf("����"); break;
        default: printf("δ֪����");
    }
}

/********************** ��ϣ��ʵ�� ************************/

#define TABLE_SIZE 101  // ��ϣ���С�����������ٳ�ͻ��
PolyNode* hashTable[TABLE_SIZE];  // ��ϣ�����飨ÿ��Ԫ��������ͷָ�룬��������ַ�������ͻ��

/**
 * ��ϣ����������ָ�������ϣ������
 * @param exp ����ʽ���ָ��
 * @return ��ϣ��������ȷ���Ǹ���
 */
int hashFunc(int exp) {
    return (exp % TABLE_SIZE + TABLE_SIZE) % TABLE_SIZE;  // ������ָ�������
}

/**
 * ���ϣ��������ʽ��Զ��ϲ�ͬ���
 * @param coef ���ϵ��
 * @param exp ���ָ��
 */
void insertHash(int coef, int exp) {
    if (coef == 0) return;  // ϵ��Ϊ0�����������
    int index = hashFunc(exp);  // ��������
    PolyNode *p = hashTable[index];  // ������Ӧ����
    while (p) {
        if (p->exp == exp) {  // �ҵ�ͬ���ָ����ͬ��
            p->coef += coef;  // �ϲ�ϵ��
            return;
        }
        p = p->next;
    }
    // δ�ҵ�ͬ��������½ڵ��������ͷ��
    PolyNode *newNode = (PolyNode*)malloc(sizeof(PolyNode));
    newNode->coef = coef;
    newNode->exp = exp;
    newNode->next = hashTable[index];  // ͷ�巨
    hashTable[index] = newNode;
}

/**
 * ��չ�ϣ���ͷ����нڵ��ڴ�
 */
void clearHashTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        PolyNode *p = hashTable[i];
        while (p) {
            PolyNode *temp = p;
            p = p->next;
            free(temp);  // �ͷŵ�ǰ�ڵ�
        }
        hashTable[i] = NULL;  // ����ͷ�ÿ�
    }
}

/**
 * qsort����ص���������ָ���������ж���ʽ��
 */
int cmpDesc(const void *a, const void *b) {
    return (*(PolyNode**)b)->exp - (*(PolyNode**)a)->exp;
}

/**
 * ��ӡ��ϣ���д洢�Ķ���ʽ����ָ������
 */
void printHash() {
    PolyNode *arr[1000];  // ��ʱ����洢���з�����
    int count = 0;
    // �ռ���ϣ�������з���ϵ������
    for (int i = 0; i < TABLE_SIZE; i++) {
        PolyNode *p = hashTable[i];
        while (p) {
            if (p->coef != 0) arr[count++] = p;
            p = p->next;
        }
    }
    // ��ָ����������
    qsort(arr, count, sizeof(PolyNode*), cmpDesc);
    // ��ӡ������
    for (int i = 0; i < count; i++) {
        printTerm(arr[i]->coef, arr[i]->exp, i == 0);
    }
    if (count == 0) printf("0");  // �ն���ʽ��ӡ0
}

/**
 * ��ϣ��ʵ�ֶ���ʽ�ӷ�������������ʽ��������ϣ���Զ��ϲ�ͬ���
 * @param poly1 ��һ������ʽ������
 * @param poly2 �ڶ�������ʽ������
 */
void addHash(PolyNode *poly1, PolyNode *poly2) {
    for (PolyNode *p = poly1; p; p = p->next) insertHash(p->coef, p->exp);
    for (PolyNode *p = poly2; p; p = p->next) insertHash(p->coef, p->exp);
}

/**
 * ��ϣ��ʵ�ֶ���ʽ������poly1 - poly2������poly1�����poly2�ĸ��
 * @param poly1 ��������ʽ
 * @param poly2 ������ʽ
 */
void subtractHash(PolyNode *poly1, PolyNode *poly2) {
    for (PolyNode *p = poly1; p; p = p->next) insertHash(p->coef, p->exp);
    for (PolyNode *p = poly2; p; p = p->next) insertHash(-p->coef, p->exp);  // ϵ��ȡ��
}

/**
 * ��ϣ��ʵ�ֶ���ʽ�˷���������ˣ�ϵ����ˣ�ָ����ӣ�
 * @param poly1 ��һ������ʽ
 * @param poly2 �ڶ�������ʽ
 */
void multiplyHash(PolyNode *poly1, PolyNode *poly2) {
    for (PolyNode *p1 = poly1; p1; p1 = p1->next) {
        for (PolyNode *p2 = poly2; p2; p2 = p2->next) {
            insertHash(p1->coef * p2->coef, p1->exp + p2->exp);  // ϵ������ָ����
        }
    }
}

/**
 * ��ϣ��ʵ�ֶ���ʽ��������֧�ֳ��Ե���ʽ��poly1 / ����ʽ��
 * @param poly1 ��������ʽ
 * @param poly2 ������ʽ�������ǵ���ʽ��
 */
void divideHash(PolyNode *poly1, PolyNode *poly2) {
    if (!poly2 || !poly2->next) {  // ����Ƿ�Ϊ����ʽ��ֻ��һ�
        int c = poly2->coef, e = poly2->exp;
        for (PolyNode *p = poly1; p; p = p->next) {
            insertHash(p->coef / c, p->exp - e);  // ϵ���̣�ָ����
        }
    } else {
        printf("��ʾ����ϣ�������֧�ֳ��Ե���ʽ\n");
    }
}

/**
 * �����ϣ��ռ�õ����ڴ�ռ�
 * @return �ܿռ䣨�ֽڣ�����ϣ������ + ���нڵ�
 */
size_t calculateHashSpace() {
    size_t space = TABLE_SIZE * sizeof(PolyNode*);  // ��ϣ�����鱾��Ŀռ�
    for (int i = 0; i < TABLE_SIZE; i++) {
        PolyNode *p = hashTable[i];
        while (p) {
            space += sizeof(PolyNode);  // ÿ���ڵ�Ŀռ�
            p = p->next;
        }
    }
    return space;
}

/********************** AVL��ʵ�� ************************/

/**
 * ��ȡAVL���ڵ�ĸ߶ȣ��սڵ�߶�Ϊ0��
 * @param node Ŀ��ڵ�
 * @return �ڵ�߶�
 */
int height(AVLNode *node) { return node ? node->height : 0; }

/**
 * ȡ�������������ֵ
 */
int max(int a, int b) { return a > b ? a : b; }

/**
 * �����µ�AVL���ڵ�
 * @param coef ϵ��
 * @param exp ָ��
 * @return �½ڵ�ָ��
 */
AVLNode* newAVLNode(int coef, int exp) {
    AVLNode *node = (AVLNode*)malloc(sizeof(AVLNode));
    node->coef = coef;
    node->exp = exp;
    node->height = 1;  // �½ڵ�߶�Ϊ1
    node->left = node->right = NULL;  // ��������Ϊ��
    return node;
}

/**
 * AVL������ת����������ƽ�������
 * @param y ʧ��ڵ�
 * @return ��ת���µĸ��ڵ㣨x��
 */
AVLNode* rightRotate(AVLNode *y) {
    AVLNode *x = y->left;       // x��y�����ӽڵ�
    AVLNode *T2 = x->right;     // T2��x��������
    x->right = y;               // y��Ϊx�����ӽڵ�
    y->left = T2;               // T2��Ϊy��������
    // ���¸߶�
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;  // x��Ϊ�µĸ��ڵ�
}

/**
 * AVL������ת����������ƽ�������
 * @param x ʧ��ڵ�
 * @return ��ת���µĸ��ڵ㣨y��
 */
AVLNode* leftRotate(AVLNode *x) {
    AVLNode *y = x->right;      // y��x�����ӽڵ�
    AVLNode *T2 = y->left;      // T2��y��������
    y->left = x;                // x��Ϊy�����ӽڵ�
    x->right = T2;              // T2��Ϊx��������
    // ���¸߶�
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;  // y��Ϊ�µĸ��ڵ�
}

/**
 * ����AVL�ڵ��ƽ�����ӣ��������߶� - �������߶ȣ�
 * @param node Ŀ��ڵ�
 * @return ƽ�����ӣ���Χ[-1,1]Ϊƽ�⣩
 */
int getBalance(AVLNode *node) { return node ? height(node->left) - height(node->right) : 0; }

/**
 * ��AVL������ڵ㣨�Զ�ƽ�⣩
 * @param node ��ǰ�ڵ㣨�ݹ������
 * @param coef �������ϵ��
 * @param exp �������ָ������Ϊ��ֵ��
 * @return �����ƽ����������ڵ�
 */
AVLNode* insertAVL(AVLNode *node, int coef, int exp) {
    // 1. ִ�б�׼BST����
    if (!node) return newAVLNode(coef, exp);  // �����������½ڵ�
    if (exp < node->exp)
        node->left = insertAVL(node->left, coef, exp);  // ָ��С�ķ�������
    else if (exp > node->exp)
        node->right = insertAVL(node->right, coef, exp);  // ָ����ķ�������
    else {
        node->coef += coef;  // ָ����ͬ���ϲ�ϵ��
        return node;
    }
    // 2. ���µ�ǰ�ڵ�߶�
    node->height = 1 + max(height(node->left), height(node->right));
    // 3. ����ƽ�����ӣ�����Ƿ�ʧ��
    int balance = getBalance(node);
    // 4. ��������ʧ�������������ת����
    // ����ʧ�⣨�����������������ߣ�
    if (balance > 1 && exp < node->left->exp)
        return rightRotate(node);
    // ����ʧ�⣨�����������������ߣ�
    if (balance < -1 && exp > node->right->exp)
        return leftRotate(node);
    // ����ʧ�⣨�����������������ߣ�
    if (balance > 1 && exp > node->left->exp) {
        node->left = leftRotate(node->left);  // ������ת������
        return rightRotate(node);  // ������ת��ǰ�ڵ�
    }
    // ����ʧ�⣨�����������������ߣ�
    if (balance < -1 && exp < node->right->exp) {
        node->right = rightRotate(node->right);  // ������ת������
        return leftRotate(node);  // ������ת��ǰ�ڵ�
    }
    return node;  // δʧ�⣬����ԭ�ڵ�
}

/**
 * ǰ�����AVL���������ڵ���뵽Ŀ��AVL����
 * @param result Ŀ��AVL����ָ���ָ�룬�����޸ĸ��ڵ㣩
 * @param node ԴAVL���ĵ�ǰ�ڵ�
 */
void preOrderInsert(AVLNode **result, AVLNode *node) {
    if (!node) return;
    *result = insertAVL(*result, node->coef, node->exp);  // ���뵱ǰ�ڵ�
    preOrderInsert(result, node->left);  // �ݹ�������
    preOrderInsert(result, node->right);  // �ݹ�������
}

/**
 * AVL��ʵ�ֶ���ʽ�ӷ����ϲ�����AVL���Ľڵ�
 * @param poly1 ��һ������ʽ��AVL����
 * @param poly2 �ڶ�������ʽ��AVL����
 * @return �������ʽ��AVL����
 */
AVLNode* addAVL(AVLNode *poly1, AVLNode *poly2) {
    AVLNode *result = NULL;
    preOrderInsert(&result, poly1);  // �����һ������ʽ�����нڵ�
    preOrderInsert(&result, poly2);  // ����ڶ�������ʽ�����нڵ㣨�Զ��ϲ�ͬ���
    return result;
}

/**
 * AVL��ʵ�ֶ���ʽ������poly1 - poly2������poly1�Ľڵ��poly2�ĸ��ڵ㣩
 * @param poly1 ��������ʽ
 * @param poly2 ������ʽ
 * @return �������ʽ
 */
AVLNode* subtractAVL(AVLNode *poly1, AVLNode *poly2) {
    AVLNode *result = NULL;
    preOrderInsert(&result, poly1);  // ����poly1�����нڵ�
    if (!poly2) return result;
    // ����poly2�Ľڵ㣨ϵ��ȡ����
    result = insertAVL(result, -poly2->coef, poly2->exp);
    // �ݹ鴦����������
    result = subtractAVL(result, poly2->left);
    result = subtractAVL(result, poly2->right);
    return result;
}

/**
 * ����������������AVL�ڵ�����һ��AVL�������нڵ���ˣ��������Ŀ����
 * @param result Ŀ������
 * @param node �����ڵ㣨���Ե�һ������ʽ��
 * @param poly �ڶ�������ʽ��AVL����
 */
void multiplyNodeWithAVL(AVLNode **result, AVLNode *node, AVLNode *poly) {
    if (!node || !poly) return;
    multiplyNodeWithAVL(result, node, poly->left);  // �ݹ�������
    // ��˺����������ϵ������ָ���ͣ�
    *result = insertAVL(*result, node->coef * poly->coef, node->exp + poly->exp);
    multiplyNodeWithAVL(result, node, poly->right);  // �ݹ�������
}

/**
 * �����������ݹ�ʵ������AVL���ĳ˷�
 * @param result Ŀ������
 * @param poly1 ��һ������ʽ
 * @param poly2 �ڶ�������ʽ
 */
void multiplyAVLHelper(AVLNode **result, AVLNode *poly1, AVLNode *poly2) {
    if (!poly1 || !poly2) return;
    multiplyAVLHelper(result, poly1->left, poly2);  // �ݹ鴦��poly1��������
    multiplyNodeWithAVL(result, poly1, poly2);  // ��ǰ�ڵ���poly2�����нڵ����
    multiplyAVLHelper(result, poly1->right, poly2);  // �ݹ鴦��poly1��������
}

/**
 * AVL��ʵ�ֶ���ʽ�˷�
 * @param poly1 ��һ������ʽ
 * @param poly2 �ڶ�������ʽ
 * @return �������ʽ
 */
AVLNode* multiplyAVL(AVLNode *poly1, AVLNode *poly2) {
    if (!poly1 || !poly2) return NULL;  // �ն���ʽ���ؿ�
    AVLNode *result = NULL;  
    multiplyAVLHelper(&result, poly1, poly2);
    return result;
}

/**
 * AVL��ʵ�ֶ���ʽ��������֧�ֳ��Ե���ʽ
 * @param poly1 ��������ʽ
 * @param poly2 ������ʽ�������ǵ���ʽ��
 * @return �������ʽ
 */
AVLNode* divideAVL(AVLNode *poly1, AVLNode *poly2) {
    // ��������ʽ�Ƿ�Ϊ����ʽ��������������
    if (!poly2 || poly2->left || poly2->right) {
        printf("��ʾ��AVL��������֧�ֳ��Ե���ʽ\n");
        return NULL;
    }
    int c = poly2->coef, e = poly2->exp;  // ����ʽ��ϵ����ָ��
    AVLNode *result = NULL;

    if (!poly1) return NULL;
    // �ݹ鴦����������
    AVLNode *left = divideAVL(poly1->left, poly2);
    AVLNode *right = divideAVL(poly1->right, poly2);
    // ����ǰ�ڵ㣨ϵ���̣�ָ���
    if (poly1->coef != 0) {
        result = insertAVL(result, poly1->coef / c, poly1->exp - e);
    }
    // �ϲ����������Ľ��
    preOrderInsert(&result, left);
    preOrderInsert(&result, right);
    return result;
}

/**
 * ����AVL��ռ�õ����ڴ�ռ�
 * @param root AVL�����ڵ�
 * @return �ܿռ䣨�ֽڣ������нڵ�Ĵ�С֮��
 */
size_t calculateAVLSpace(AVLNode *root) {
    if (!root) return 0;
    // �ݹ���㣺��ǰ�ڵ��С + ��������С + ��������С
    return sizeof(AVLNode) + calculateAVLSpace(root->left) + calculateAVLSpace(root->right);
}

/**
 * �ͷ�AVL�������нڵ��ڴ棨���������
 * @param node ���ڵ�
 */
void freeAVL(AVLNode *node) {
    if (!node) return;
    freeAVL(node->left);   // �ͷ�������
    freeAVL(node->right);  // �ͷ�������
    free(node);            // �ͷŵ�ǰ�ڵ�
}

/**
 * ���������������������-��-�󣩴�ӡAVL����ȷ����ָ���������
 * @param root ��ǰ�ڵ�
 * @param isFirst ��ʶ�Ƿ�Ϊ��һ�ָ�봫�ݣ����ڿ�ݹ��޸ģ�
 */
void printAVLHelper(AVLNode *root, int *isFirst) {
    if (!root) return;
    printAVLHelper(root->right, isFirst);  // �ȴ�ӡ��������ָ������
    printTerm(root->coef, root->exp, *isFirst);  // ��ӡ��ǰ�ڵ�
    *isFirst = 0;  // ��������ǵ�һ��
    printAVLHelper(root->left, isFirst);   // �ٴ�ӡ��������ָ����С��
}

/**
 * ��ӡAVL���д洢�Ķ���ʽ
 * @param root AVL�����ڵ�
 */
void printAVL(AVLNode *root) {
    int isFirst = 1;  // ��ʼ��Ϊ��һ��
    printAVLHelper(root, &isFirst);
    if (isFirst) printf("0");  // �ն���ʽ��ӡ0
}

/********************** ����/ת������ ************************/

/**
 * ��ӡ����Ķ���ʽ��������ʽ������ָ����������
 * @param poly ����ʽ����ͷ
 */
void printInputPoly(PolyNode *poly) {
    if (!poly) {
        printf("0");
        return;
    }
    int isFirst = 1;
    PolyNode *arr[100];  // ��ʱ����洢������
    int count = 0;
    while (poly) {
        arr[count++] = poly;
        poly = poly->next;
    }
    // ð�����򣺰�ָ������
    for (int i = 0; i < count-1; i++) {
        for (int j = i+1; j < count; j++) {
            if (arr[i]->exp < arr[j]->exp) {
                PolyNode *temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
    // ��ӡ��������
    for (int i = 0; i < count; i++) {
        printTerm(arr[i]->coef, arr[i]->exp, isFirst);
        isFirst = 0;
    }
}

/**
 * �ֶ��������ʽ��������ʽ��
 * @param termCount ����������洢�������
 * @return ����ʽ����ͷ
 */
PolyNode* inputPoly(int *termCount) {
    int n;
    printf("���������ʽ����: ");
    scanf("%d", &n);
    *termCount = n;
    PolyNode *head = NULL;
    for (int i = 0; i < n; i++) {
        int c, e;
        printf("�������%d�� (ϵ�� ָ��): ", i + 1);
        scanf("%d%d", &c, &e);
        // �����ڵ㣬ͷ�巨��������
        PolyNode *newNode = (PolyNode*)malloc(sizeof(PolyNode));
        newNode->coef = c;
        newNode->exp = e;
        newNode->next = head;
        head = newNode;
    }
    // ��ʾ����Ķ���ʽ�������
    printf("����Ķ���ʽ: ");
    printInputPoly(head);
    printf("\n");
    return head;
}

/**
 * ������ɶ���ʽ��������ʽ��
 * @param termCount ����
 * @param maxCoef ϵ�����ֵ������ֵ��
 * @param maxExp ָ�����ֵ
 * @return ����ʽ����ͷ
 */
PolyNode* generatePoly(int termCount, int maxCoef, int maxExp) {
    PolyNode *head = NULL;
    for (int i = 0; i < termCount; i++) {
        PolyNode *node = (PolyNode*)malloc(sizeof(PolyNode));
        // ���ϵ����1~maxCoef��50%����Ϊ��
        node->coef = (rand() % maxCoef) + 1;
        if (rand() % 2 == 0) {
            node->coef = -node->coef;
        }
        // ���ָ����0~maxExp
        node->exp = rand() % (maxExp + 1);
        // ͷ�巨��������
        node->next = head;
        head = node;
    }
    return head;
}

/**
 * ������ʽ����ת��ΪAVL��
 * @param list ����ʽ����ͷ
 * @return AVL�����ڵ�
 */
AVLNode* listToAVL(PolyNode *list) {
    AVLNode *root = NULL;
    while (list) {
        root = insertAVL(root, list->coef, list->exp);  // �������AVL��
        list = list->next;
    }
    return root;
}

/********************** ������ ************************/

int main() {
    while (1) {  // ��ѭ����ѡ��ģʽ
        printf("\n==== ����ʽ����ϵͳ ====\n");
        printf("1. ����ģʽ\n");
        printf("2. ��������ģʽ\n");
        printf("0. �˳�\n");
        printf("��ѡ��ģʽ: ");
        int mode;
        scanf("%d", &mode);

        if (mode == 0) {
            printf("�������˳�\n");
            break;
        }

        if (mode == 1) {
            // ========= ����ģʽ���ֶ��������ʽ������ =========
            while (1) {
                int n1, n2;  // ��������ʽ������
                printSeparator();
                
                // �����һ������ʽ
                printf("��һ������ʽ��\n");
                PolyNode *poly1 = inputPoly(&n1);
                
                // ����ڶ�������ʽ
                printf("\n�ڶ�������ʽ��\n");
                PolyNode *poly2 = inputPoly(&n2);
                
                // ѡ�����㣨���ڶ����ǵ���ʽ���Զ�ѡ�������
                printSeparator();
                int isDivision = (n2 == 1);  // ����ʽ�ж�
                int op = 0;
                if (!isDivision) {
                    printf("��ѡ�����㣺1.�ӷ� 2.���� 3.�˷�\n");
                    scanf("%d", &op);
                } else {
                    printf("��⵽�ڶ�������ʽ�ǵ���ʽ���Զ�ѡ���������\n");
                    op = 4;  // ����
                }
                printSeparator();
                
                // ��ʾ��ǰ����
                printf("��ǰ���㣺 ");
                printInputPoly(poly1);
                printf(" ");
                switch(op) {
                    case 1: printf("+"); break;
                    case 2: printf("-"); break;
                    case 3: printf("��"); break;
                    case 4: printf("��"); break;
                }
                printf(" ");
                printInputPoly(poly2);
                printf("\n");
                printSeparator();
                
                // ��ϣ�����㣨��ʱ+��������
                for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;  // ��ʼ����ϣ��
                double hash_start = getTimeMs();  // ��ʼ��ʱ
                if (op == 1) addHash(poly1, poly2);
                else if (op == 2) subtractHash(poly1, poly2);
                else if (op == 3) multiplyHash(poly1, poly2);
                else divideHash(poly1, poly2);
                double hash_end = getTimeMs();  // ������ʱ
                double hashTime = hash_end - hash_start;  // ��ʱ
                size_t hashSpace = calculateHashSpace();  // �ռ�ռ��
                
                // AVL�����㣨��ʱ+��������
                AVLNode *poly1_avl = listToAVL(poly1);  // ת��ΪAVL��
                AVLNode *poly2_avl = listToAVL(poly2);
                double avl_start = getTimeMs();  // ��ʼ��ʱ
                AVLNode *result_avl = NULL;
                if (op == 1) result_avl = addAVL(poly1_avl, poly2_avl);
                else if (op == 2) result_avl = subtractAVL(poly1_avl, poly2_avl);
                else if (op == 3) result_avl = multiplyAVL(poly1_avl, poly2_avl);
                else result_avl = divideAVL(poly1_avl, poly2_avl);
                double avl_end = getTimeMs();  // ������ʱ
                double avlTime = avl_end - avl_start;  // ��ʱ
                size_t avlSpace = calculateAVLSpace(result_avl);  // �ռ�ռ��
                
                // ������ܶԱȽ��
                printf("===== �������Ա� =====\n");
                printf("| ���ݽṹ | ������ | ��ʱ (ms) | �ڴ�ռ�� (B) |\n");
                printf("|----------|----------|-----------|--------------|\n");
                
                // ��ϣ����
                printf("| ��ϣ��   | ");
                printHash();
                printf(" | %10.6f | %12zu |\n", hashTime, hashSpace);
                
                // AVL�����
                printf("| AVL��    | ");
                printAVL(result_avl);
                printf(" | %10.6f | %12zu |\n", avlTime, avlSpace);
                printSeparator();
                
                // �ͷ��ڴ棨��ֹй©��
                while (poly1) { PolyNode *t = poly1; poly1 = poly1->next; free(t); }
                while (poly2) { PolyNode *t = poly2; poly2 = poly2->next; free(t); }
                freeAVL(poly1_avl); 
                freeAVL(poly2_avl); 
                freeAVL(result_avl);
                clearHashTable();
                
                // ѡ��������˳�����ģʽ
                int cont;
                printf("�Ƿ��������ģʽ������ 1 ���������� 0 �������˵�: ");
                scanf("%d", &cont);
                if (cont == 0) break;
            }
        }

        if (mode == 2) {
            // ========= ��������ģʽ���Զ����ɶ���ʽ��������㲢ͳ������ =========
            int termCount1, termCount2, maxCoef, maxExp, testRuns;
            
            // ��ȡ���Բ���
            printf("�������������Բ�����\n");
            printf("��һ������ʽ����: ");
            scanf("%d", &termCount1);
            printf("�ڶ�������ʽ����: ");
            scanf("%d", &termCount2);
            printf("���ϵ��ֵ: ");
            scanf("%d", &maxCoef);
            printf("���ָ��ֵ: ");
            scanf("%d", &maxExp);
            printf("���Դ���: ");
            scanf("%d", &testRuns);
            
            // ��ʼ�����������
            srand(time(NULL));
            
            // �洢ÿ�β��Եĺ�ʱ
            double hashAddTimes[testRuns], avlAddTimes[testRuns];
            double hashSubTimes[testRuns], avlSubTimes[testRuns];
            double hashMulTimes[testRuns], avlMulTimes[testRuns];
            double hashDivTimes[testRuns], avlDivTimes[testRuns];
            
            // ���ڴ�ռ�ã����ڼ���ƽ��ֵ��
            size_t totalHashSpace = 0, totalAVLSpace = 0;
            
            printf("\n��ʼ�������ԣ��� %d ��...\n", testRuns);
            
            for (int run = 0; run < testRuns; run++) {
                // �Զ�������������ʽ������
                PolyNode *poly1 = generatePoly(termCount1, maxCoef, maxExp);
                PolyNode *poly2 = generatePoly(termCount2, maxCoef, maxExp);
                
                // ת��ΪAVL��
                AVLNode *poly1_avl = listToAVL(poly1);
                AVLNode *poly2_avl = listToAVL(poly2);
                
                // ���ɳ����õĵ���ʽ
                PolyNode *mono = generatePoly(1, maxCoef, maxExp);  // ����ʽ��1�
                AVLNode *mono_avl = listToAVL(mono);
                
                // ====== ���Լӷ� ======
                for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;
                double h_start = getTimeMs(); 
                addHash(poly1, poly2); 
                double h_end = getTimeMs();
                hashAddTimes[run] = h_end - h_start;
                totalHashSpace += calculateHashSpace();
                clearHashTable();  // ��չ�ϣ��
                
                double a_start = getTimeMs(); 
                AVLNode *r1 = addAVL(poly1_avl, poly2_avl); 
                double a_end = getTimeMs();
                avlAddTimes[run] = a_end - a_start;
                totalAVLSpace += calculateAVLSpace(r1);
                freeAVL(r1);  // �ͷŽ����
                
                // ====== ���Լ��� ======
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
                
                // ====== ���Գ˷� ======
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
                
                // ====== ���Գ����������Ե���ʽ�� ======
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
                
                // �ͷű��β��Ե��ڴ�
                while (poly1) { PolyNode *t = poly1; poly1 = poly1->next; free(t); }
                while (poly2) { PolyNode *t = poly2; poly2 = poly2->next; free(t); }
                while (mono) { PolyNode *t = mono; mono = mono->next; free(t); }
                freeAVL(poly1_avl); 
                freeAVL(poly2_avl); 
                freeAVL(mono_avl);
                
                // ��ʾ����
                printf("��ɲ��� %d/%d\n", run + 1, testRuns);
            }
            
            // ����ƽ����ʱ
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
            
            // ����������Խ��
            printSeparator();
            printf("===== �������Խ����ƽ��ֵ�� =====\n");
            printf("���Բ�����\n");
            printf("  - ����ʽ1����: %d\n", termCount1);
            printf("  - ����ʽ2����: %d\n", termCount2);
            printf("  - ���ϵ��ֵ:  %d\n", maxCoef);
            printf("  - ���ָ��ֵ:  %d\n", maxExp);
            printf("  - ���Դ���:    %d\n", testRuns);
            printSeparator();
            
            // �ӷ����
            printf("--- �ӷ����� ---\n");
            printf("| ���ݽṹ | ƽ����ʱ (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| ��ϣ��   | %13.6f |\n", hashAddAvg);
            printf("| AVL��    | %13.6f |\n", avlAddAvg);
            printSeparator();
            
            // �������
            printf("--- �������� ---\n");
            printf("| ���ݽṹ | ƽ����ʱ (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| ��ϣ��   | %13.6f |\n", hashSubAvg);
            printf("| AVL��    | %13.6f |\n", avlSubAvg);
            printSeparator();
            
            // �˷����
            printf("--- �˷����� ---\n");
            printf("| ���ݽṹ | ƽ����ʱ (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| ��ϣ��   | %13.6f |\n", hashMulAvg);
            printf("| AVL��    | %13.6f |\n", avlMulAvg);
            printSeparator();
            
            // �������
            printf("--- �������� (poly1 �� ����ʽ) ---\n");
            printf("| ���ݽṹ | ƽ����ʱ (ms) |\n");
            printf("|----------|---------------|\n");
            printf("| ��ϣ��   | %13.6f |\n", hashDivAvg);
            printf("| AVL��    | %13.6f |\n", avlDivAvg);
            printSeparator();
            
            // �ڴ�ռ�ý�����ܿռ� / ���Դ��� / 4�����㣩
            printf("--- �ڴ�ռ�ã�ƽ��ֵ�� ---\n");
            printf("| ���ݽṹ | ƽ���ڴ�ռ�� (B) |\n");
            printf("|----------|------------------|\n");
            printf("| ��ϣ��   | %16.2f |\n", (double)totalHashSpace / (testRuns * 4));
            printf("| AVL��    | %16.2f |\n", (double)totalAVLSpace / (testRuns * 4));
            printSeparator();
        }
    }
    return 0;
}
