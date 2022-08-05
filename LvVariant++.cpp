// 
// LabVIEW Variant++ DSO/DLL library 
// Author:	Danny Holstein
// Desc:	Allows "Labview Variant" to "C++ variant" data exchange.  Will create a C++ variant reference
//			from the LabVIEW "to flattened string" VI (with type array, which includes type, name and value).  Also,  
//			convert C++ variant reference to LabVIEW flattened data for conversion into LabVIEW Variant
#define VARIANT_LVPP_VERSION "Variant_LVPP-0.1"

#include <list>     //  container of generated objects for error checking (avoid SEGFAULT)
#include <variant>  //  container for variant data
#include <iostream> //  std::cout
#include <inttypes.h>
#include <memory.h>

using namespace std;

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
void LV_str_cp(LStrHandle LV_string, string c_str)
{
    DSSetHandleSize(LV_string, sizeof(int) + c_str.length() * sizeof(char));
    (*LV_string)->cnt = c_str.length();
    memcpy((char*)(*LV_string)->str, &(c_str[0]), c_str.length());
}
void LV_str_cp(LStrHandle LV_string, char* c_str, int size)
{
    DSSetHandleSize(LV_string, sizeof(int) + size * sizeof(char));
    (*LV_string)->cnt = size;
    memcpy((char*)(*LV_string)->str, c_str, size);
}
void LV_str_cat(LStrHandle LV_string, string str)    //  concatenate C++ str to LSTR LV_string
{
    int n = (*LV_string)->cnt;
    DSSetHandleSize(LV_string, sizeof(int) + n + str.length());
    (*LV_string)->cnt = n + str.length();
    memcpy((char*)(*LV_string)->str + n, str.c_str(), str.length());
}
LStrHandle LVStr(string str)    //  convert string to new LV string handle
{
    if (str.length() == 0) return NULL;
    LStrHandle l; if ((l = (LStrHandle) DSNewHClr(sizeof(int32) + str.length())) == NULL) return NULL;
    memmove((char*)(*l)->str, str.c_str(), ((*l)->cnt = str.length()));
    return l;
}
LStrHandle LVStr(string str, int size)
{
    if (size == 0) return NULL;
    LStrHandle l; if ((l = (LStrHandle)DSNewHClr(sizeof(int32) + size)) == NULL) return NULL;
    memmove((char*)(*l)->str, str.c_str(), ((*l)->cnt = size));
    return l;
}
LStrHandle LVStr(char* str, int size)
{
    if (size == 0) return NULL;
    LStrHandle l; if ((l = (LStrHandle)DSNewHClr(sizeof(int32) + size)) == NULL) return NULL;
    memmove((char*)(*l)->str, str, ((*l)->cnt = size));
    return l;
}
//class LvType {public: enum e;}
#endif //   LabVIEW stuff

#define VAR_TYPES int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float, double, string*, uint8_t*
uint8_t GetLVTD(int idx) {
    uint8_t types[] = {TD::I8, TD::U8, TD::I16, TD::U16, TD::U32, TD::I32, TD::SGL, TD::DBL, TD::String, TD::Array};
    return types[idx];
}
#define MAGIC 0x13131313    //  doesn't necessarily need to be unique to this, specific library

static std::list<string> myVariantsStr, myVariantsNm;   //  where we store the actual strings and names

class VarObj
{
public:
    void* addr;
    bool IsNull = false;    //  NULL variants are important, DB and XML results can often return NULL
    string *name = NULL, *str = NULL;
    variant<VAR_TYPES> data;
    VarObj(string n, int32_t d) {data = d; addr = this; if (n.length() > 0) name = new string(n);}
    VarObj(string n, bool SetNull)
        {IsNull = SetNull; addr = (SetNull ? this : NULL); if (n.length() > 0) name = new string(n);}
    VarObj(string n,  char* d, int sz) {
        data = (string*) NULL; addr = this; if (sz > 0) str = new string(d, sz);
        if (n.length() > 0) name = new string(n);}
    ~VarObj() {delete name; delete str;}

    bool operator< (const VarObj rhs) const { return addr < rhs.addr; }
    bool operator<= (const VarObj rhs) const { return addr <= rhs.addr; }
    bool operator== (const VarObj rhs) const { return addr == rhs.addr; }
    uint32_t canary_end = MAGIC;  //  check for buffer overrun/corruption
};
static std::list<VarObj*> myVariants;

static string ObjectErrStr; //  where we store user-checked/non-API error messages
static bool   ObjectErr;    //  set to "true" for user-checked/non-API error messages

bool IsVariant(VarObj* addr) //  check for corruption/validity, use <list> to track all open connections, avoid SEGFAULT
{
    if (addr == NULL) { ObjectErrStr = "NULL Variant"; ObjectErr = true; return false; }
    bool b = false;
    for (auto i : myVariants) { if (i->addr == addr) {b = true; break;}}
    if (!b) { ObjectErrStr = "Invalid Variant (unallocated memory or non-variant reference)"; ObjectErr = true; return false; }

    if (addr->addr == addr && addr->canary_end == MAGIC) { ObjectErr = false; ObjectErrStr = "SUCCESS"; return true; }
    else { ObjectErr = true; ObjectErrStr = "Variant memory corrupted"; return false; }
}

struct TStrS {uInt8 fill; uInt8 size; uInt8 nm; uInt8 type;
              union {char c[0]; struct {uInt16 ff[2]; char c[0];} s;} c;}; //  match LabVIEW "Type String" structure/array

extern "C" {  //  functions to be called from LabVIEW.  'extern "C"' is necessary to prevent overload name mangling

void* ToVariant(U8ArrayHdl TypeStr, LStrHandle Data, LStrHandle Image, bool GetImage) {
    if ((**TypeStr).dimSize == 0)
        {ObjectErr = true; ObjectErrStr = "NULL Type String"; return NULL;}
    ObjectErr = false; ObjectErrStr = "SUCCESS";

    TStrS *tStr = (TStrS*) (**TypeStr).elt;
    string nm; bool named = tStr->nm & 0x40;
    VarObj *V = NULL; char* LvName = (tStr->type == TD::String? (char*) tStr->c.s.c : (char*) tStr->c.c);
    if (!named) nm = "";
    else nm = string((char*) &(LvName[1]), LvName[0]);
    
    switch (tStr->type)
    {
    case TD::Void:    //  note, LabVIEW drops names on std flatten, though user can add
        V = new VarObj(nm, true); break;
    case TD::I32:
        V = new VarObj(nm, *((int32_t*)  (**Data).str)); break;
    case TD::String:
        V = new VarObj(nm, (char*) &((**Data).str[4]), *((int*) (**Data).str)); break;
    default:
        ObjectErr = true; ObjectErrStr = "Unsupported data type: " + (tStr->type);
        break;
    }
    if (V != NULL)
        {myVariants.push_back(V);if (GetImage) LV_str_cp(Image, string((char*) V, sizeof(VarObj)));}
    return V;
}

#define LvTypeDecriptor(A) ((uInt16) (A->IsNull ? 0 : GetLVTD(A->data.index())))

int FromVariant(VarObj* LvVarObj, U8ArrayHdl TypeStr, LStrHandle Data, bool del) {
    if (!IsVariant(LvVarObj)) return -1;
//  NmSz:   Size of LV type string, in bytes, though type string is cast by LabVIEW to I16 (padded up)
//          It includes the standard 4 byte header, an extra 4 0xff for strings, then the byte length of
//          the variant name plus the name itself.
    uInt8 NmSz = (LvVarObj->name == NULL? 0: LvVarObj->name->length() + 2 - LvVarObj->name->length() % 2)
                + 4 + (LvTypeDecriptor(LvVarObj) == TD::String ? 4 : 0);
    DSSetHandleSize(TypeStr, sizeof(int32) + NmSz);
    TStrS* tStr = (TStrS*) (**TypeStr).elt;
    char* LvName;
    if (LvTypeDecriptor(LvVarObj) == TD::String) LvName = tStr->c.s.c;  //  offset accomodated here
    else  LvName = tStr->c.c;
    switch LvTypeDecriptor(LvVarObj)    //  handle convertion to LV type strings
    {
    case TD::Void:
    case TD::I8: case TD::U8: case TD::I16: case TD::U16: case TD::U32: case TD::I32:
    case TD::String:
        (**TypeStr).dimSize = NmSz;
        tStr = (TStrS*)(**TypeStr).elt;
        tStr->fill = 0x00; tStr->size = NmSz; tStr->type = LvTypeDecriptor(LvVarObj);
        if (LvVarObj->name != NULL)
            {tStr->nm =  0x40; LvName[0] = LvVarObj->name->length();
            memcpy(&LvName[1], LvVarObj->name->c_str(), LvName[0]);}
        if (LvTypeDecriptor(LvVarObj) == TD::String) tStr->c.s.ff[0] = tStr->c.s.ff[1] = 0xffff;
        break;
    default:
        ObjectErr = true; ObjectErrStr = "Unsupported data type: " + (LvTypeDecriptor(LvVarObj));
        return -1;
        break;
    }

    switch (LvTypeDecriptor(LvVarObj))    //  handle convertion to LV data strings
    {
    case TD::Void:
        break;
    case TD::I8: case TD::U8:
        LV_str_cp(Data, string((char*) & LvVarObj->data, sizeof(int8))); break;
    case TD::I16: case TD::U16:
        LV_str_cp(Data, string((char*) & LvVarObj->data, sizeof(int16))); break;
    case TD::U32: case TD::I32:
        LV_str_cp(Data, string((char*) & LvVarObj->data, sizeof(int32))); break;
    case TD::String:
        int sz; sz = 0; 
        if (LvVarObj->str != NULL)
           {sz = LvVarObj->str->length();
            LV_str_cp(Data, (char*) &sz, sizeof(int)); LV_str_cat(Data, *(LvVarObj->str));}
        else LV_str_cp(Data, (char*) &sz, sizeof(int));
        break;
    default:
        ObjectErr = true; ObjectErrStr = "Unsupported data type: " + (LvTypeDecriptor(LvVarObj));
        return -1;
        break;
    }
    if (del) myVariants.remove(LvVarObj);
    return 0;
}

int DeleteVariant(VarObj* LvVarObj) {
    if (!IsVariant(LvVarObj)) return -1;
    myVariants.remove(LvVarObj); return 0;
}

#if 1    //  the following are utility-ish functions
void GetError(VarObj* LvVarObj, tLvVarErr* error) { //  get error info from variant object properties
    if (LvVarObj == NULL || ObjectErr) {
        if (!ObjectErr) return; //  no error
        error->errnum = -1;
        error->errstr = LVStr(ObjectErrStr);
        ObjectErr = false; ObjectErrStr = "NO ERROR"; //  Clear error, but race conditions may exist, if so, da shit has hit da fan. 
    }
}

int SizeOfBool() {return sizeof(bool);}

void Version(char* buf, int sz) {
    memcpy(buf, string(VARIANT_LVPP_VERSION).c_str(), min<int>(sz, string(VARIANT_LVPP_VERSION).length()));}

#endif
}
