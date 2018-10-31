//==========================================================
// tsqlite3.h
//==========================================================
// SQLite3におけるジェネリックマクロの提供
// ---------------------------------------------------------
// 依存関係 :
// ・sqlite3.h   [子]
// ・storectrl.h [親]
//==========================================================


#ifndef _TSQLITE3_H_
#define _TSQLITE3_H_

#define STRICT


/* Standard API Header */
#include "sqlite3.h"


#ifdef UNICODE

#define _tsqlite3_open			sqlite3_open16
#define _tsqlite3_errmsg		sqlite3_errmsg16
#define _tsqlite3_prepare		sqlite3_prepare16
#define _tsqlite3_bind_text		sqlite3_bind_text16
#define _tsqlite3_column_name	sqlite3_column_name16
#define _tsqlite3_column_text	sqlite3_column_text16

#else

#define _tsqlite3_open			sqlite3_open
#define _tsqlite3_errmsg		sqlite3_errmsg
#define _tsqlite3_prepare		sqlite3_prepare
#define _tsqlite3_bind_text		sqlite3_bind_text
#define _tsqlite3_column_name	sqlite3_column_name
#define _tsqlite3_column_text	sqlite3_column_text

#endif

#endif