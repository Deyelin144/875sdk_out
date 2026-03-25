/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef guJSON__h
#define guJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* project version */
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 2
#define CJSON_VERSION_PATCH 1

/* returns the version of guJSON as a string */
extern const char* guJSON_Version(void);

#include <stddef.h>

/* guJSON Types: */
#define guJSON_False  (1 << 0)
#define guJSON_True   (1 << 1)
#define guJSON_NULL   (1 << 2)
#define guJSON_Number (1 << 3)
#define guJSON_String (1 << 4)
#define guJSON_Array  (1 << 5)
#define guJSON_Object (1 << 6)
#define guJSON_Raw    (1 << 7) /* raw json */

#define guJSON_IsReference 256
#define guJSON_StringIsConst 512

/* The guJSON structure: */
typedef struct guJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct guJSON *next;
    struct guJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct guJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==guJSON_String  and type == guJSON_Raw */
    char *valuestring;
    /* The item's number, if type==guJSON_Number */
    int valueint;
    /* The item's number, if type==guJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} guJSON;

typedef struct guJSON_Hooks
{
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} guJSON_Hooks;

/* Supply malloc, realloc and free functions to guJSON */
extern void guJSON_InitHooks(guJSON_Hooks* hooks);


/* Supply a block of JSON, and this returns a guJSON object you can interrogate. Call guJSON_Delete when finished. */
extern guJSON *guJSON_Parse(const char *value);
/* Render a guJSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *guJSON_Print(const guJSON *item);
/* Render a guJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *guJSON_PrintUnformatted(const guJSON *item);
/* Render a guJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
extern char *guJSON_PrintBuffered(const guJSON *item, int prebuffer, int fmt);
/* Render a guJSON entity to text using a buffer already allocated in memory with length buf_len. Returns 1 on success and 0 on failure. */
extern int guJSON_PrintPreallocated(guJSON *item, char *buf, const int len, const int fmt);
/* Delete a guJSON entity and all subentities. */
extern void   guJSON_Delete(guJSON *c);

/* Returns the number of items in an array (or object). */
extern int	  guJSON_GetArraySize(const guJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern guJSON *guJSON_GetArrayItem(const guJSON *array, int item);
/* Get item "string" from object. Case insensitive. */
extern guJSON *guJSON_GetObjectItem(const guJSON *object, const char *string);
extern int guJSON_HasObjectItem(const guJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when guJSON_Parse() returns 0. 0 when guJSON_Parse() succeeds. */
extern const char *guJSON_GetErrorPtr(void);

/* These calls create a guJSON item of the appropriate type. */
extern guJSON *guJSON_CreateNull(void);
extern guJSON *guJSON_CreateTrue(void);
extern guJSON *guJSON_CreateFalse(void);
extern guJSON *guJSON_CreateBool(int b);
extern guJSON *guJSON_CreateNumber(double num);
extern guJSON *guJSON_CreateString(const char *string);
/* raw json */
extern guJSON *guJSON_CreateRaw(const char *raw);
extern guJSON *guJSON_CreateArray(void);
extern guJSON *guJSON_CreateObject(void);

/* These utilities create an Array of count items. */
extern guJSON *guJSON_CreateIntArray(const int *numbers, int count);
extern guJSON *guJSON_CreateFloatArray(const float *numbers, int count);
extern guJSON *guJSON_CreateDoubleArray(const double *numbers, int count);
extern guJSON *guJSON_CreateStringArray(const char **strings, int count);

/* Append item to the specified array/object. */
extern void guJSON_AddItemToArray(guJSON *array, guJSON *item);
extern void	guJSON_AddItemToObject(guJSON *object, const char *string, guJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the guJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & guJSON_StringIsConst) is zero before
 * writing to `item->string` */
extern void	guJSON_AddItemToObjectCS(guJSON *object, const char *string, guJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing guJSON to a new guJSON, but don't want to corrupt your existing guJSON. */
extern void guJSON_AddItemReferenceToArray(guJSON *array, guJSON *item);
extern void	guJSON_AddItemReferenceToObject(guJSON *object, const char *string, guJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern guJSON *guJSON_DetachItemFromArray(guJSON *array, int which);
extern void   guJSON_DeleteItemFromArray(guJSON *array, int which);
extern guJSON *guJSON_DetachItemFromObject(guJSON *object, const char *string);
extern void   guJSON_DeleteItemFromObject(guJSON *object, const char *string);

/* Update array items. */
extern void guJSON_InsertItemInArray(guJSON *array, int which, guJSON *newitem); /* Shifts pre-existing items to the right. */
extern void guJSON_ReplaceItemInArray(guJSON *array, int which, guJSON *newitem);
extern void guJSON_ReplaceItemInObject(guJSON *object,const char *string,guJSON *newitem);

/* Duplicate a guJSON item */
extern guJSON *guJSON_Duplicate(const guJSON *item, int recurse);
/* Duplicate will create a new, identical guJSON item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error. If not, then guJSON_GetErrorPtr() does the job. */
extern guJSON *guJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

extern void guJSON_Minify(char *json);

/* Macros for creating things quickly. */
#define guJSON_AddNullToObject(object,name) guJSON_AddItemToObject(object, name, guJSON_CreateNull())
#define guJSON_AddTrueToObject(object,name) guJSON_AddItemToObject(object, name, guJSON_CreateTrue())
#define guJSON_AddFalseToObject(object,name) guJSON_AddItemToObject(object, name, guJSON_CreateFalse())
#define guJSON_AddBoolToObject(object,name,b) guJSON_AddItemToObject(object, name, guJSON_CreateBool(b))
#define guJSON_AddNumberToObject(object,name,n) guJSON_AddItemToObject(object, name, guJSON_CreateNumber(n))
#define guJSON_AddStringToObject(object,name,s) guJSON_AddItemToObject(object, name, guJSON_CreateString(s))
#define guJSON_AddRawToObject(object,name,s) guJSON_AddItemToObject(object, name, guJSON_CreateRaw(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define guJSON_SetIntValue(object,val) ((object) ? (object)->valueint = (object)->valuedouble = (val) : (val))
#define guJSON_SetNumberValue(object,val) ((object) ? (object)->valueint = (object)->valuedouble = (val) : (val))

/* Macro for iterating over an array */
#define guJSON_ArrayForEach(pos, head) for(pos = (head)->child; pos != NULL; pos = pos->next)

#ifdef __cplusplus
}
#endif

#endif
