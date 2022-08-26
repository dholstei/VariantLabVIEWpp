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
#include "LvVariant++.h"

using namespace std;

#if 1   //  LabVIEW stuff
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
#endif //   LabVIEW stuff

#define VAR_TYPES int8, uInt8, int16, uInt16, int32, uInt32, float, double, string*, uint8_t*
uint8_t GetLVTD(int idx) {
    uint8_t types[] = {TD::I8, TD::U8, TD::I16, TD::U16, TD::I32, TD::U32, TD::SGL, TD::DBL, TD::String, TD::Array};
    return types[idx];
}
#define MAGIC 0x13131313    //  random/unique, non 0x00000000 and 0xffffffff number

VarObj::VarObj(std::string n, bool SetNull)
{IsNull = SetNull; addr = (SetNull ? this : NULL); if (n.length() > 0) name = new std::string(n); }
VarObj::VarObj(std::string n, int8_t   d) { data = d; addr = this; if (n.length() > 0) name = new std::string(n); }
VarObj::VarObj(std::string n, uint8_t  d) { data = d; addr = this; if (n.length() > 0) name = new std::string(n); }
VarObj::VarObj(std::string n, int16_t  d) { data = d; addr = this; if (n.length() > 0) name = new std::string(n); }
VarObj::VarObj(std::string n, uint16_t d) { data = d; addr = this; if (n.length() > 0) name = new std::string(n); }
VarObj::VarObj(std::string n, int32_t  d) { data = d; addr = this; if (n.length() > 0) name = new std::string(n); }
VarObj::VarObj(std::string n, uint32_t d) { data = d; addr = this; if (n.length() > 0) name = new std::string(n); }
VarObj::VarObj(string n, float  d) { data = (float)d; addr = this; if (n.length() > 0) name = new string(n); }
VarObj::VarObj(string n, double d) { data = (double)d; addr = this; if (n.length() > 0) name = new string(n); }
VarObj::VarObj(std::string n, char* d, int sz) {
    data = (std::string*)NULL; addr = this; if (sz > 0) str = new std::string(d, sz);
    if (n.length() > 0) name = new std::string(n);
}
VarObj::~VarObj() { delete name; delete str; delete errstr; }

void VarObj::SetError(int number, std::string str)
{errnum = number; if (str.length() > 0) errstr = new std::string(str);}

VarObj* VarObj::operator= (bool SetNull) { data = SetNull; return (VarObj*) addr; }
VarObj* VarObj::operator= (int8_t   d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (uint8_t  d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (int16_t  d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (uint16_t d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (int32_t  d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (uint32_t d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (float    d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (double   d) { data = d;  return (VarObj*) addr; }
VarObj* VarObj::operator= (string* d)
    { delete str; if (d->size() > 0) str = d;  return (VarObj*) addr; }

bool VarObj::operator< (const VarObj rhs) const { return addr < rhs.addr; }
bool VarObj::operator<= (const VarObj rhs) const { return addr <= rhs.addr; }
bool VarObj::operator== (const VarObj rhs) const { return addr == rhs.addr; }

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
    case TD::I8:
        V = new VarObj(nm, (int8) * ((int8*)(**Data).str)); break;
    case TD::U8:
        V = new VarObj(nm, (uInt8) * ((uInt8*)(**Data).str)); break;
    case TD::I16:
        V = new VarObj(nm, (int16) *  ((int16*) (**Data).str)); break;
    case TD::U16:
        V = new VarObj(nm, (uInt16) * ((uInt16*)(**Data).str)); break;
    case TD::I32:
        V = new VarObj(nm, (int32_t) *  ((int32*) (**Data).str)); break;
    case TD::U32:
        V = new VarObj(nm, (uint32_t) * ((uInt32*)(**Data).str)); break;
    case TD::SGL:
        V = new VarObj(nm, (float) * ((float*)(**Data).str)); break;
    case TD::DBL:
        V = new VarObj(nm, (double) * ((double*)(**Data).str)); break;
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

int AssignVal(VarObj* LvVarObj, U8ArrayHdl TypeStr, LStrHandle Data) {
    if (!IsVariant(LvVarObj)) return -1;
    if ((**TypeStr).dimSize == 0)
        {LvVarObj->errnum = -1; LvVarObj->errstr = new string("NULL data in assignment"); return -1;}
    TStrS *tStr = (TStrS*) (**TypeStr).elt;
    switch (tStr->type)
    {
    case TD::Void:
         break;
    case TD::I8:
        LvVarObj = (VarObj*) ((int8*)(**Data).str); break;
    case TD::U8:
        LvVarObj = (VarObj*) ((uInt8*)(**Data).str); break;
    case TD::I16:
        LvVarObj = (VarObj*) ((int16*)(**Data).str); break;
    case TD::U16:
        LvVarObj = (VarObj*) ((uInt16*)(**Data).str); break;
    case TD::I32:
        LvVarObj = (VarObj*) ((int32*)(**Data).str); break;
    case TD::U32:
        LvVarObj = (VarObj*) ((uInt32*)(**Data).str); break;
    case TD::SGL:
        LvVarObj = (VarObj*) ((float*)(**Data).str); break;
    case TD::DBL:
        LvVarObj = (VarObj*) ((double*)(**Data).str); break;
    case TD::String:
        LvVarObj = (VarObj*) new string((char*)&((**Data).str[4]), *((int*)(**Data).str)); break;
    default:
        {LvVarObj->errnum = -1; LvVarObj->errstr = new string("Invalid data type in assignment"); return -1; }
        break;
    }
    return -1;
}

#define LvTypeDecriptor(A) ((uInt8) (A->IsNull ? 0 : GetLVTD(A->data.index())))

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
    case TD::SGL: case TD::DBL:
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
    case TD::U32: case TD::I32: case TD::SGL:
        LV_str_cp(Data, string((char*) & LvVarObj->data, sizeof(int32))); break;
    case TD::DBL:
        LV_str_cp(Data, string((char*)&LvVarObj->data, sizeof(double))); break;
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
    if (del) {myVariants.remove(LvVarObj); delete LvVarObj;}
    return 0;
}

int DeleteVariant(VarObj* LvVarObj) {
    if (!IsVariant(LvVarObj)) return -1;
    myVariants.remove(LvVarObj); delete LvVarObj; return 0;
}

#if 1    //  the following are utility-ish functions
void GetError(VarObj* LvVarObj, tLvVarErr* error) { //  get error info from variant object properties
    if (LvVarObj == NULL || ObjectErr) {
        if (!ObjectErr) return; //  no error
        error->errnum = -1;
        error->errstr = LVStr(ObjectErrStr);
        ObjectErr = false; ObjectErrStr = "NO ERROR"; //  Clear error, but race conditions may exist, if so, da shit has hit da fan. 
    }
    else
    {
        error->errnum = LvVarObj->errnum;
        error->errstr = LVStr(*(LvVarObj->errstr));
        LvVarObj->errnum = 0; delete LvVarObj->errstr;
    }
}

int SizeOfBool() {return sizeof(bool);}

void Version(char* buf, int sz) {
    memcpy(buf, string(VARIANT_LVPP_VERSION).c_str(), min<int>(sz, string(VARIANT_LVPP_VERSION).length()));}

#endif
}
