#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_LEN     2048
#define MAX_KEYS    20
#define MAX_SEGS    10
#define MAX_SUBPAGES 100

#define HA_OFFSET_ERROR 0xFFFFFFFFFFFFFFFF

#define KEY_ALG_BTREE       1
#define KEY_ALG_RTREE       2
#define KEY_ALG_HASH        3
#define KEY_ALG_FULLTEXT    4
char *keyAlgName[5] = {
    "UNKNOWN",
    "BTREE",
    "RTREE",
    "HASH",
    "FULLTEXT"
};

#define KEY_TYPE_TEXT           1
#define KEY_TYPE_BINARY         2
#define KEY_TYPE_SHORT_INT      3
#define KEY_TYPE_LONG_INT       4
#define KEY_TYPE_FLOAT          5
#define KEY_TYPE_DOUBLE         6
#define KEY_TYPE_NUM            7
#define KEY_TYPE_USHORT_INT     8
#define KEY_TYPE_ULONG_INT      9
#define KEY_TYPE_LONGLONG       10
#define KEY_TYPE_ULONGLONG      11
#define KEY_TYPE_INT24          12
#define KEY_TYPE_UINT24         13
#define KEY_TYPE_INT8           14
#define KEY_TYPE_VARTEXT1       15
#define KEY_TYPE_VARBINARY1     16
#define KEY_TYPE_VARTEXT2       17
#define KEY_TYPE_VARBINARY2     18
#define KEY_TYPE_BIT            19

char *keyTypeName[20] = {
    "UNKNOWN",
    "TEXT",
    "BINARY",
    "SHORT_INT",
    "LONG_INT",
    "FLOAT",
    "DOUBLE",
    "NUM",
    "USHORT_INT",
    "ULONG_INT",
    "LONGLONG",
    "ULONGLONG",
    "INT24",
    "UINT24",
    "INT8",
    "VARTEXT1",
    "VARBINARY1",
    "VARTEXT2",
    "VARBINARY2",
    "BIT"
};

struct segdef {
    uint8_t type;
    uint8_t maybeNull;
    uint16_t len;
};

struct keydef {
    uint64_t offset;
    uint16_t len;
    uint16_t blockLen;
    uint8_t  segs;
    uint8_t  alg;
    struct segdef segdef[MAX_SEGS];
};

struct MYI_Header
{
    uint16_t len;
    uint8_t  keys;

    uint64_t keyFileLen;
    uint64_t dataFileLen;

    uint16_t basePos;
    uint32_t fields;
    uint32_t recordLen;
    uint8_t  recordRefLen;
    uint8_t  keyRefLen;

    uint32_t updateCount;
    uint64_t records;
    uint64_t recordsDeleted;
    uint64_t dellink;

    struct keydef keydef[MAX_KEYS];
};

int fd;
uint8_t buf[BUF_LEN];
struct MYI_Header header;

void eat(uint16_t n)
{
    if (n > BUF_LEN) {
        fprintf(stderr, "eat(n) should < %d\n", BUF_LEN);
        exit(1);
    }

    ssize_t nBytes = read(fd, buf, n);
    if (nBytes == n) {
        return;
    }

    if (nBytes == -1) {
        perror("read");
        exit(1);
    }

    fprintf(stderr, "expects read %d bytes, actually read %ld", n, nBytes);
    exit(1);
}

uint16_t buf2MysqlUint16()
{
    return (buf[0] << 8 | buf[1]);
}

uint32_t buf2MysqlUint24()
{
    return (  buf[0] << 16
            | buf[1] << 8
            | buf[2]);
}

uint32_t buf2MysqlUint32()
{
    return (  buf[0] << 24
            | buf[1] << 16
            | buf[2] << 8
            | buf[3]);
}

uint64_t buf2MysqlUint40()
{
    return (  (uint64_t)buf[0] << 32
            | buf[1] << 24
            | buf[2] << 16
            | buf[3] << 8
            | buf[4]);
}

uint64_t buf2MysqlUint48()
{
    return (  (uint64_t)buf[0] << 40
            | (uint64_t)buf[1] << 32
            | buf[2] << 24
            | buf[3] << 16
            | buf[4] << 8
            | buf[5]);
}

uint64_t buf2MysqlUint64()
{
    return (  (uint64_t)buf[0] << 56
            | (uint64_t)buf[1] << 48
            | (uint64_t)buf[2] << 40
            | (uint64_t)buf[3] << 32
            | buf[4] << 24
            | buf[5] << 16
            | buf[6] << 8
            | buf[7]);
}

void printKeyValue(int *offset, struct keydef *keydef)
{
    int i, j, varcharLen;
    struct segdef *segdef;

    for (i = 0; i < keydef->segs; i++) {
        if (i > 0) {
            printf("|");
        }
        segdef = keydef->segdef + i;
        if (segdef->maybeNull) {
            eat(1);
            *offset += 1;
            if (buf[0] == 0) {
                printf("(    NULL 00) ");
                continue;
            } else {
                printf("(NOT NULL %02x) ", buf[0]);
            }
        }
        switch (segdef->type) {
            case KEY_TYPE_TEXT:
                eat(segdef->len);
                *offset += segdef->len;
                printf("%.*s", segdef->len, buf);
                break;
            case KEY_TYPE_USHORT_INT:
                eat(segdef->len);
                *offset += segdef->len;
                printf("%u", buf2MysqlUint16());
                break;
            case KEY_TYPE_UINT24:
                eat(segdef->len);
                *offset += segdef->len;
                printf("%u", buf2MysqlUint24());
                break;
            case KEY_TYPE_ULONG_INT:
                eat(segdef->len);
                *offset += segdef->len;
                printf("%u", buf2MysqlUint32());
                break;
            case KEY_TYPE_ULONGLONG:
                eat(segdef->len);
                *offset += segdef->len;
                printf("%lu", buf2MysqlUint64());
                break;
            case KEY_TYPE_VARTEXT1:
                eat(2);
                *offset += 2;
                varcharLen = buf[1];
                eat(varcharLen);
                *offset += varcharLen;
                printf("%.*s", varcharLen, buf);
                break;
            default:
                eat(segdef->len);
                *offset += segdef->len;
                for (j = 0; j < segdef->len; j++) {
                    printf("%02x ", buf[j]);
                }
                break;
        }
    }

    printf(" -> ");
    eat(header.recordRefLen);
    *offset += header.recordRefLen;
    for (j = 0; j < header.recordRefLen; j++) {
        printf("%02x ", buf[j]);
    }
    printf("\n");
}

uint64_t calcKeyRef(keyRefLen)
{
    switch (header.keyRefLen) {
        case 5:
            return buf2MysqlUint40();
        case 6:
            return buf2MysqlUint48();
        default:
            fprintf(stderr, "unsupport keyRefLen: %d\n", header.keyRefLen);
            exit(1);
    }
}

void printBtreeNode(struct keydef *keydef)
{
    eat(2);
    uint16_t blockHeader  = buf2MysqlUint16();
    int      isLeaf       = (blockHeader & 0x8000) == 0;
    uint16_t keyValuesLen = (blockHeader & 0x7FFF) - 2;

    int offset = 0;
    if (isLeaf) {
        printf("BTREE leaf, values total length = %d\n", keyValuesLen);
        int num = 0;
        while (offset < keyValuesLen) {
            num++;
            printf("%d: ", num);
            printKeyValue(&offset, keydef);
        }
        printf("\n");
        return;
    }

    printf("BTREE node, values total length = %d\n", keyValuesLen);

    uint64_t subPageOffset[MAX_SUBPAGES];
    int pageCount = 0;

    eat(header.keyRefLen);
    offset += header.keyRefLen;
    subPageOffset[pageCount] = calcKeyRef() * keydef->blockLen;
    printf("child %d offset = %#lx\n", pageCount, subPageOffset[pageCount]);
    pageCount++;

    while (offset < keyValuesLen) {
        printKeyValue(&offset, keydef);

        eat(header.keyRefLen);
        offset += header.keyRefLen;
        subPageOffset[pageCount] = calcKeyRef() * keydef->blockLen;
        printf("child %d offset = %#lx\n", pageCount, subPageOffset[pageCount]);
        pageCount++;
    }
    printf("\n");

    int i;
    for (i = 0; i < pageCount; i++) {
        printf("child %d:\n", i);
        lseek(fd, subPageOffset[i], SEEK_SET);
        printBtreeNode(keydef);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int i, j;
    struct keydef *keydef;
    struct segdef *segdef;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s /path/to/table.MYI\n", argv[0]);
        return 0;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 0;
    }

    // state
    eat(6);
    eat(2);
    header.len = buf2MysqlUint16();
    eat(4);
    eat(2);
    header.basePos = buf2MysqlUint16();
    eat(2+2);
    eat(1);
    header.keys = buf[0];
    if (header.keys > MAX_KEYS) {
        fprintf(stderr, "索引数量超出处理能力\n");
        exit(1);
    }
    eat(5+4);
    eat(8);
    header.records = buf2MysqlUint64();
    eat(8);
    header.recordsDeleted = buf2MysqlUint64();
    eat(8);
    eat(8);
    header.dellink = buf2MysqlUint64();
    eat(8);
    header.keyFileLen = buf2MysqlUint64();
    eat(8);
    header.dataFileLen = buf2MysqlUint64();
    eat(8*4+4*3);
    eat(4);
    header.updateCount = buf2MysqlUint32();

    for (i = 0; i < header.keys; i++) {
        eat(8);
        header.keydef[i].offset = buf2MysqlUint64();
    }

    // base
    lseek(fd, header.basePos, SEEK_SET);
    eat(8*5+4);
    eat(4);
    header.recordLen = buf2MysqlUint32();
    eat(4*4);
    eat(4);
    header.fields = buf2MysqlUint32();
    eat(4);
    eat(1);
    header.recordRefLen = buf[0];
    eat(1);
    header.keyRefLen = buf[0];
    eat(2+2*5+2+2+4+6);

    // keydef
    for (i = 0; i < header.keys; i++) {
        eat(2);
        uint8_t segs = buf[0];
        if (segs > MAX_SEGS) {
            fprintf(stderr, "索引字段数超出处理能力\n");
            exit(1);
        }
        uint8_t alg = buf[1];
        if (alg != KEY_ALG_BTREE) {
            fprintf(stderr, "当前只能处理Btree索引\n");
            exit(1);
        }
        keydef = header.keydef + i;
        keydef->segs = segs;
        keydef->alg  = alg;
        eat(2);
        eat(2);
        keydef->blockLen = buf2MysqlUint16();
        eat(2);
        keydef->len = buf2MysqlUint16();
        eat(4);

        for (j = 0; j < segs; j++) {
            segdef = keydef->segdef + j;
            eat(1);
            segdef->type = buf[0];
            eat(1);
            eat(1);
            segdef->maybeNull = buf[0];
            eat(3+2);
            eat(2);
            segdef->len = buf2MysqlUint16();
            eat(8);
        }
    }

    printf(".MYI Header分4部分: state, base, keydef, recinfo\n");
    printf("\n");

    printf("第1部分state包含以下信息:\n");
    printf(
        "这个数据库表经过 %u 次操作后,有 %lu 条有效的记录, %lu 条已删除的记录", 
        header.updateCount, header.records, header.recordsDeleted
    );
    if (header.recordsDeleted) {
        printf(",第1条已删除的记录位置.MYD文件的 %#lx(%lu) 字节处", header.dellink, header.dellink);
    }
    printf("\n");
    printf(
        "索引文件.MYI的大小为 %lu 字节,数据文件.MYD的大小为 %lu 字节\n"
        "整个header的长度为 %#x(%u) 个字节,base部分从文件的 %#x(%u) 字节处开始\n"
        "这个表定义了 %u 个索引:\n",
        header.keyFileLen, header.dataFileLen,
        header.len, header.len, header.basePos, header.basePos,
        header.keys
    );
    for (i = 0; i < header.keys; i++) {
        if (header.keydef[i].offset == HA_OFFSET_ERROR) {
            printf("    第%d个索引位于 由于表中尚无数据,所以索引为空\n", i+1);
        } else {
            printf("    第%d个索引位于 %#lx(%lu) 字节处\n", i+1, header.keydef[i].offset, header.keydef[i].offset);
        }
    }
    printf("\n");

    printf("第2部分base包含以下信息:\n");
    printf(
        "这个数据库表每条记录的长度是 %d 个字节,共有 %d 个字段, recordRefLen = %d, keyRefLen = %d\n"
        "注意: 由于表中可能有null字段,因此record header的长度可能为1,也可能多于1,所以记录长度要大于直接加和计算的结果\n",
        header.recordLen, header.fields - 1, header.recordRefLen, header.keyRefLen
    );
    printf("\n");

    printf("第3部分keydef包含以下信息:\n");
    for (i = 0; i < header.keys; i++) {
        keydef = header.keydef + i;
        printf(
            "第%d个索引包含 %d 个字段,索引类型为 %s, blockLen = %d, keyLen = %d\n",
            i+1, keydef->segs, keyAlgName[keydef->alg], keydef->blockLen, keydef->len
        );
        for (j = 0; j < keydef->segs; j++) {
            segdef = keydef->segdef + j;
            printf(
                "    字段%d的类型是 %s, 长度为 %d, null_bit = %d\n",
                j+1, keyTypeName[segdef->type], segdef->len, segdef->maybeNull
            );
        }
    }
    printf("\n");

    printf("第4部分recinfo目前没有我们感兴趣的信息,跳过\n\n");

    printf("====KEY VALUES====\n\n");
    for (i = 0; i < header.keys; i++) {
        printf("@@第 %d 个索引@@\n\n", i+1);

        keydef = header.keydef + i;
        if (keydef->offset == HA_OFFSET_ERROR) {
            printf("key_root = HA_OFFSET_ERROR, skip\n");
            continue;
        }

        lseek(fd, keydef->offset, SEEK_SET);
        printBtreeNode(keydef);
    }

    return 1;
}
