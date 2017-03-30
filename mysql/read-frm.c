#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

struct fileinfo {
    uint8_t engineType;
    uint16_t recordLen;
    uint16_t keybuffOffset;
    uint16_t keybuffLen;
    uint16_t keyInfoLen;
    uint16_t forminfoOffset;
};

#define MAX_KEYS 20
#define MAX_SEGS 10
#define MAX_COLS 100

struct seg {
    uint16_t fieldnr;
    uint16_t offset;
    uint16_t type;
    uint16_t len;
};

struct key {
    char     *name;
    uint16_t flags;
    uint16_t len;
    uint8_t  segsCount;
    uint8_t  alg;
    uint16_t blockSize;
    struct seg segs[MAX_SEGS];
};

struct keybuff {
    uint8_t totalKeys;
    uint8_t totalSegs;
    uint16_t nameAndCommentLen;
    struct key keys[MAX_KEYS];
};

struct str {
    uint16_t len;
    char *s;
};

struct forminfo {
    struct str comment;
    uint16_t columns;
    uint16_t screenBuffLen;
};

struct columns {
    char *name;
    uint8_t flags;
};

struct field {
    char *name;
    uint8_t row;
    uint8_t col;
    uint8_t scLen;
    uint16_t displayLen;
    uint32_t recordPos;
    uint16_t packFlag;
    uint8_t  uniregCheck;
    uint8_t  intervalId;
    uint8_t  sqlType;
    uint16_t charset;
    uint16_t commentLen;
};

struct packFields {
    struct field fields[MAX_COLS];
};

#define ENGINE_TYPE_MYISAM 9
#define ENGINE_TYPE_INNODB 12

#define NAMES_SEP_CHAR  0xff

#define FIELDFLAG_DECIMAL       1
#define FIELDFLAG_NO_DEFAULT    16384
#define FIELDFLG_MAYBE_NULL     32768

#define FIELD_NR_MASK   16383

enum enum_field_types {
MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
MYSQL_TYPE_BIT,
MYSQL_TYPE_NEWDECIMAL=246,
MYSQL_TYPE_ENUM=247,
MYSQL_TYPE_SET=248,
MYSQL_TYPE_TINY_BLOB=249,
MYSQL_TYPE_MEDIUM_BLOB=250,
MYSQL_TYPE_LONG_BLOB=251,
MYSQL_TYPE_BLOB=252,
MYSQL_TYPE_VAR_STRING=253,
MYSQL_TYPE_STRING=254,
MYSQL_TYPE_GEOMETRY=255
};
char *sqlTypeNames[17] = {
    "decimal",
    "tinyint",
    "short",
    "long",
    "float",
    "double",
    "null",
    "timestamp",
    "longlong",
    "int24",
    "date",
    "time",
    "datetime",
    "year",
    "newDate",
    "varchar",
    "bit"
};
char *sqlTypeNames2[] = {
    "newDecimal",
    "enum",
    "set",
    "tinyBlob",
    "mediumBlob",
    "longBlob",
    "blob",
    "varString",
    "string",
    "Geometry"
};
char *sqlType(uint8_t type)
{
    if (type < 17) {
        return sqlTypeNames[type];
    }

    if (type >= 246 && type <= 255) {
        return sqlTypeNames2[type - 246];
    }

    return "unknown";
}

char *charsetName(uint16_t charset)
{
    switch (charset) {
        case 33:
            return "utf8_general_ci";
        case 8:
            return "latin1_swedish_ci";
        default:
            return "unknown";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/table.frm\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat buf;
    if (fstat(fd, &buf) == -1) {
        perror("fstat");
        return 1;
    }

    uint8_t *fp = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (fp == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // fileinfo
    struct fileinfo fileinfo;
    fileinfo.engineType     = fp[3];
    fileinfo.recordLen      = *((uint16_t *)(fp + 16));
    fileinfo.keybuffOffset  = *((uint16_t *)(fp + 6));
    fileinfo.keybuffLen     = *((uint16_t *)(fp + 47));
    fileinfo.keyInfoLen     = *((uint16_t *)(fp + 28));
    /*
        forminfo的位置依赖其自身的一个数据

        filepos = make_new_entry();
        maxlength=(uint) next_io_size((ulong) (uint2korr(forminfo)+1000));
        int2store(forminfo+2,maxlength);
        int4store(fileinfo+10,(ulong) (filepos+maxlength));
        mysql_file_seek(file, filepos, MY_SEEK_SET, MYF(0));
        mysql_file_write(file, forminfo, 288, MYF_RW)

        所以forminfo的位置 = fileinfo[10] - forminfo[2] = fileinfo[10] - maxlength = fileinfo[10] - 0x1000
    */
    fileinfo.forminfoOffset = *((uint16_t *)(fp + 10)) - 0x1000;

    // keybuff
    uint8_t *p = fp + fileinfo.keybuffOffset;
    struct keybuff keybuff;
    keybuff.totalKeys = p[0];
    if (keybuff.totalKeys > MAX_KEYS) {
        fprintf(stderr, "索引数量超出处理能力\n");
        return 1;
    }
    keybuff.totalSegs = p[1];
    keybuff.nameAndCommentLen = *((uint16_t *)(p + 4));
    p += 6;

    int i, j;
    for (i = 0; i < keybuff.totalKeys; i++) {
        keybuff.keys[i].flags       = *((uint16_t *)p);
        keybuff.keys[i].len         = *((uint16_t *)(p+2));
        keybuff.keys[i].segsCount   = p[4];
        if (keybuff.keys[i].segsCount > MAX_SEGS) {
            fprintf(stderr, "单个索引中包含的字段数超出处理能力\n");
            return 1;
        }
        keybuff.keys[i].alg         = p[5];
        keybuff.keys[i].blockSize   = *((uint16_t *)(p+6));
        p += 8;
        for (j = 0; j < keybuff.keys[i].segsCount; j++) {
            keybuff.keys[i].segs[j].fieldnr = (*((uint16_t *)p) & FIELD_NR_MASK);
            keybuff.keys[i].segs[j].offset  = *((uint16_t *)(p+2));
            keybuff.keys[i].segs[j].type    = *((uint16_t *)(p+5));
            keybuff.keys[i].segs[j].len     = *((uint16_t *)(p+7));
            p += 9;
        }
    }
    p++;
    for (i = 0; i < keybuff.totalKeys; i++) {
        keybuff.keys[i].name = (char *)p;
        j = 0;
        while (*p != NAMES_SEP_CHAR) {
            p++;
            j++;
        }
        keybuff.keys[i].name = strndup(keybuff.keys[i].name, j);
        p++;
    }
    // ignore key comment

    // forminfo
    struct forminfo forminfo;
    p = fp + fileinfo.forminfoOffset;
    forminfo.comment.len = p[46];
    if (forminfo.comment.len != 255) {
        forminfo.comment.s = strndup((char *)(p + 47), forminfo.comment.len);
    }
    forminfo.columns = *((uint16_t *)(p + 258));
    if (forminfo.columns > MAX_COLS) {
        fprintf(stderr, "数据库表的字段数超出处理能力\n");
        return 1;
    }
    forminfo.screenBuffLen = *((uint16_t *)(p + 260));


    printf("这个数据库表的engine是 ");
    switch (fileinfo.engineType) {
        case ENGINE_TYPE_MYISAM:
            printf("MyISAM");
            break;
        case ENGINE_TYPE_INNODB:
            printf("InnoDB");
            break;
        default:
            printf("NOT MyISAM, NOT InnoDB");
            break;
    }
    printf("\n");
    printf("recordLen = %d\n", fileinfo.recordLen);
    printf(
        "索引定义位于文件的 %#x 字节处, 索引在文件中占了 %#x 字节, 其中有效数据占了 %#x 字节\n"
        "其它的是padding的数据\n\n",
        fileinfo.keybuffOffset, fileinfo.keybuffLen, fileinfo.keyInfoLen
    );

    printf("这个表共有 %d 个索引, 这些索引共包含了 %d 个字段\n", keybuff.totalKeys, keybuff.totalSegs);
    struct key *key;
    struct seg *seg;
    for (i = 0; i < keybuff.totalKeys; i++) {
        key = keybuff.keys + i;
        printf(
            "%d-%s: flags = %#x, len = %d, segs = %d\n",
            i+1, key->name, key->flags, key->len, key->segsCount
        );
        for (j = 0; j < key->segsCount; j++) {
            seg = key->segs + j;
            printf(
                "   字段%d: fieldnr = %#x, offset = %#x, type = %d, len = %d\n",
                j+1,
                seg->fieldnr, seg->offset, seg->type, seg->len
            );
        }
    }
    printf("\n");

    printf(
        "在keybuff之后,offset=%#x处,是一条空记录,长度为 %d 个字节,记录里的字段都取默认值\n", 
        fileinfo.keybuffOffset + fileinfo.keybuffLen,
        fileinfo.recordLen
    );
    p = fp + fileinfo.keybuffOffset + fileinfo.keybuffLen;
    j = 0;
    for (i = 0; i < fileinfo.recordLen; i++) {
        printf("%02x ", p[i]);
        j++;
        if (j == 8) {
            printf(" ");
        } else if (j == 16) {
            printf("\n");
            j = 0;
        }
    }
    printf("\n\n");
    p += fileinfo.recordLen;

    struct str connectStr;
    connectStr.len = *((uint16_t *)p);
    if (connectStr.len) {
        connectStr.s = strndup((char *)(p+2), connectStr.len);
        printf("connect_string = %s\n", connectStr.s);
    } else {
        printf("connect_string is EMPTY\n");
    }
    p += 2 + connectStr.len;

    struct str dbType;
    dbType.len = *((uint16_t *)p);
    dbType.s   = strndup((char *)(p+2), dbType.len);
    printf("db_type = %s\n", dbType.s);
    p += 2 + dbType.len + 6;

    // 假设key没有parser_name

    if (forminfo.comment.len == 255) {
        forminfo.comment.len = *((uint16_t *)p);
        forminfo.comment.s   = strndup((char *)(p+2), forminfo.comment.len);
        p += 2 + forminfo.comment.len;
    }
    if (forminfo.comment.len) {
        printf("table comment = %s (len = %d)\n", forminfo.comment.s, forminfo.comment.len);
    } else {
        printf("table comment is EMPTY\n");
    }
    printf("\n");

    // 假设tablespace_length = 0

    // pack_fields
    p = fp + fileinfo.forminfoOffset + 288 + forminfo.screenBuffLen;
    struct packFields packFields;
    for (i = 0; i < forminfo.columns; i++) {
        packFields.fields[i].row   = p[0];
        packFields.fields[i].col   = p[1];
        packFields.fields[i].scLen = p[2];
        packFields.fields[i].displayLen = *((uint16_t *)(p+3));
        packFields.fields[i].recordPos = (*((uint32_t *)(p+5)) >> 8);
        packFields.fields[i].packFlag = *((uint16_t *)(p+8));
        packFields.fields[i].uniregCheck = p[10];
        packFields.fields[i].intervalId = p[12];
        packFields.fields[i].sqlType = p[13];
        packFields.fields[i].charset = (p[11] << 8 | p[14]);
        packFields.fields[i].commentLen = *((uint16_t *)(p+15));
        p += 17; // FCOMP
    }
    p++;
    for (i = 0; i < forminfo.columns; i++) {
        packFields.fields[i].name = (char *)p;
        j = 0;
        while (*p != NAMES_SEP_CHAR) {
            p++;
            j++;
        }
        packFields.fields[i].name = strndup(packFields.fields[i].name, j);
        p++;
    }

    printf("这个数据库表共有 %d 个字段\n", forminfo.columns);
    struct field *field;
    for (i = 0; i < forminfo.columns; i++) {
        field = packFields.fields + i;
        printf(
            "%d-%s: %s(%d), charset = %s\n"
            "           packFlag = %#x (NULL = %s, unsigned = %s, defaultValue = %s)\n"
            "           commentLen = %d\n",
            i+1, field->name, sqlType(field->sqlType), field->displayLen,
            charsetName(field->charset),
            field->packFlag,
            (field->packFlag & FIELDFLG_MAYBE_NULL ? "YES" : "NO"),
            (field->packFlag & FIELDFLAG_DECIMAL ? "NO" : "YES"),
            (field->packFlag & FIELDFLAG_NO_DEFAULT ? "None" : "Has"),
            field->commentLen
        );
    }
    printf("\n");

    printf("Q: default value的值保存在哪里了?\n");
    printf("A: .frm里保存了一条空的记录,default value都在这了\n");

    return 0;
}
