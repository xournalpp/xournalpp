/*
 * Xournal++
 *
 * Type macros like GLib use for C to find wrong pointers at runtime
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


#ifndef __XOURNALTYPE_H__
#define __XOURNALTYPE_H__

#define XOJ_DECLARE_TYPE(name, id) \
	const int __XOJ_TYPE_ ## name = id

/**
 * This creates a Xournal mTypeinformation Attribute
 */
#define XOJ_TYPE_ATTRIB \
	int __xoj_type; \
	int __xoj_typeCheckvalue

/**
 * Initalize the Xournal type info, this should be called in the constructor
 */
#define XOJ_INIT_TYPE(name) \
		this->__xoj_type = __XOJ_TYPE_ ## name; \
		this->__xoj_typeCheckvalue = 0xFFAA00AA

/**
 * Release the Xournal type info, this should be called in the destructor
 */
#define XOJ_RELEASE_TYPE(name) \
		XOJ_CHECK_TYPE(name) \
		this->__xoj_type = -(__XOJ_TYPE_ ## name); \
		this->__xoj_typeCheckvalue = 0xFFAA00AA


/**
 * Checks the type of "this" and returns if "this" is not an instance of "name"
 */
#define XOJ_CHECK_TYPE_OBJ(obj, name) \
		if(obj == NULL) { \
			g_warning("XojTypeCheck failed: NULL %s:%i", __FILE__, __LINE__);\
		} \
		if(obj->__xoj_typeCheckvalue != 0xFFAA00AA) { \
			g_warning("XojTypeCheck failed: expected %s but get something else on %s:%i", #name, __FILE__, __LINE__);\
		} \
		if(obj->__xoj_type != __XOJ_TYPE_ ## name) { \
			g_warning("XojTypeCheck failed: expected %s but get %s on %s:%i", #name, xoj_type_getName(obj->__xoj_type), __FILE__, __LINE__);\
		}

/**
 * Checks the type of "this" and returns if "this" is not an instance of "name"
 */
#define XOJ_CHECK_TYPE_OBJ_RET(obj, name, ret) \
		if(obj == NULL) { \
			g_warning("XojTypeCheck failed: NULL %s:%i", __FILE__, __LINE__);\
		} \
		if(obj->__xoj_typeCheckvalue != 0xFFAA00AA) { \
			g_warning("XojTypeCheck failed: expected %s but get something else on %s:%i", #name, __FILE__, __LINE__);\
		} \
		if(obj->__xoj_type != __XOJ_TYPE_ ## name) { \
			g_warning("XojTypeCheck failed: expected %s but get %s on %s:%i", #name, xoj_type_getName(obj->__xoj_type), __FILE__, __LINE__);\
		}

/**
 * Checks the type of "this" and returns if "this" is not an instance of "name"
 */
#define XOJ_CHECK_TYPE(name) \
		if(this == NULL) { \
			g_warning("XojTypeCheck failed: NULL %s:%i", __FILE__, __LINE__);\
		} \
		if(this->__xoj_typeCheckvalue != 0xFFAA00AA) { \
			g_warning("XojTypeCheck failed: expected %s but get something else on %s:%i", #name, __FILE__, __LINE__);\
		} \
		if(this->__xoj_type != __XOJ_TYPE_ ## name) { \
			g_warning("XojTypeCheck failed: expected %s but get %s on %s:%i", #name, xoj_type_getName(this->__xoj_type), __FILE__, __LINE__);\
		}
/**
 * Checks the type of "this" and returns "ret" if "this" is not an instance of "name"
 */
#define XOJ_CHECK_TYPE_RET(name, ret) \
		if(this == NULL) { \
			g_warning("XojTypeCheck failed: NULL %s:%i", __FILE__, __LINE__);\
		} \
		if(this->__xoj_typeCheckvalue != 0xFFAA00AA) { \
			g_warning("XojTypeCheck failed: expected %s but get something else on %s:%i", #name, __FILE__, __LINE__);\
		} \
		if(this->__xoj_type != __XOJ_TYPE_ ## name) { \
			g_warning("XojTypeCheck failed: expected %s but get %s on %s:%i", #name, xoj_type_getName(this->__xoj_type), __FILE__, __LINE__);\
		}

const char * xoj_type_getName(int id);

#include "XournalTypeList.h"

#endif /* __XOURNALTYPE_H__ */
