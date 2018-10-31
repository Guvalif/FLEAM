//==========================================================
// type.h
//==========================================================
// 各種情報のグループ化構造体の提供
// ---------------------------------------------------------
// 依存関係 :
// ・sqlite3.h   [子]
// ・cmdproc.h   [親]
// ・storectrl.h [親]
//==========================================================


#ifndef _TYPE_H_
#define _TYPE_H_

#define STRICT
#define WIN32_LEAN_AND_MEAN


/* Windows32 API Header */
#include <windows.h>

/* Standard API Header */
#include "sqlite3.h"


/* Flag Information Type */
typedef struct {
	BOOL Init;
	BOOL Mgr;
} FLAG_INFO;


/* Stock Information Type */
typedef struct {
	LPTSTR Tag;
	int Value[3];
	int Stock;
} STOCK_INFO;


/* Time(YDHM) Information Type */
typedef struct {
	int Month;
	int Day;
	int Hour;
	int Minute;
} TIME_INFO;


/* Stteing Information Type */
typedef struct {
	int Tax;
	BOOL AddTax;
	TIME_INFO Time[3];
	double TimeNum[3];
	int ProfitSUM[3];
	int TaxSUM[3];
} SETTING_INFO;


/* File Infomation Type */
typedef struct {
	sqlite3* DB;
	sqlite3* Setting;
	HANDLE SystemLog;
	HANDLE SaleLog;
} FILE_INFO;

#endif