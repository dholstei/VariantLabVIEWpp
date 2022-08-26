#pragma once

#define VARIANT_LVPP_VERSION "Variant_LVPP-0.2"

#include <list>     //  container of generated objects for error checking (avoid SEGFAULT)
#include <variant>  //  container for variant data
#include <iostream> //  std::cout
#include <inttypes.h>
#include <memory.h>

using namespace std;

#define MAGIC 0x13131313    //  random/unique, non 0x00000000 and 0xffffffff number
#define VAR_TYPES int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float, double, std::string*

#if 1   //  LabVIEW stuff
#include "extcode.h"
typedef struct {
    long errnum;
    LStrHandle errstr;
    LStrHandle errdata;
} tLvVarErr;
typedef struct {
    int32_t dimSize;
    uint8_t elt[1];
} U8Array;
typedef U8Array** U8ArrayHdl;
#include "LvTypeDescriptors.h"
#define LStrString(A) string((char*) (*A)->str, (*A)->cnt)
void LV_str_cp(LStrHandle LV_string, string c_str);
void LV_str_cp(LStrHandle LV_string, char* c_str, int size);
void LV_str_cat(LStrHandle LV_string, string str);    //  concatenate C++ str to LSTR LV_string;
LStrHandle LVStr(string str);    //  convert string to new LV string handle;
LStrHandle LVStr(string str, int size);
LStrHandle LVStr(char* str, int size);
#endif //   LabVIEW stuff

class VarObj
{
public:
    void* addr;
    bool IsNull = false;    //  NULL variants are important, DB and XML results can often return NULL
    std::string* name = NULL, * str = NULL;
    variant<VAR_TYPES> data;
    int errnum = 0; std::string* errstr = NULL;  //  user-defined, object-specific error info (like invalid)

    VarObj(std::string n, bool SetNull) ;
    VarObj(std::string n, int8_t   d) ;
    VarObj(std::string n, uint8_t  d) ;
    VarObj(std::string n, int16_t  d) ;
    VarObj(std::string n, uint16_t d) ;
    VarObj(std::string n, int32_t  d) ;
    VarObj(std::string n, uint32_t d);
    VarObj(std::string n, float    d);
    VarObj(std::string n, double   d);
    VarObj(std::string n, char* d, int sz) ;
    ~VarObj() ;

    void SetError(int number, std::string str);

    bool operator< (const VarObj rhs) const ;
    bool operator<= (const VarObj rhs) const ;
    bool operator== (const VarObj rhs) const ;
    uint32_t canary_end = MAGIC;  //  check for buffer overrun/corruption
};

bool IsVariant(VarObj* addr); //  check for corruption/validity, use <list> to track all open connections, avoid SEGFAULT
