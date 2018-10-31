//==========================================================
// resource.h
//==========================================================
// 各種リソースID及びタイトルの管理
// ---------------------------------------------------------
// 依存関係 :
// ・cmdproc.h   [親]
// ・cmnctrl.h   [親]
// ・resource.rc [親]
// ・storectrl.h [親]
//==========================================================


#ifndef _RESOURCE_H_
#define _RESOURCE_H_

//==========================================================
// 各種マクロ
//==========================================================
// WINAPI 用
#define APP_NAME _T("FLE@M")
#define APP_TITLE _T("FLE@M Ver 1.08")
#define APP_TITLE_MGR _T("FLE@M Ver 1.08 [MGR]")

// RESOURCE 用
#define APP_TITLE_RES "FLE@M Ver 1.08 [MGR]"
#define APP_VERSION_RES "Version 1.08"


//==========================================================
//  ダイアログリソース
//==========================================================
// ダイアログ IDD_DLGMAIN
#define	IDC_BUTTON100	100 // SELS Button
#define	IDC_BUTTON101	101 // INFO Button
#define	IDC_EDIT102		102 // [Command Line]
#define	IDC_EDIT103		103 // [Tag]
#define	IDC_EDIT104		104 // [Quantity]
#define	IDC_LABEL105	105
#define	IDC_LABEL106	106
#define	IDC_LABEL107	107
#define	IDC_LABEL108	108
#define	IDC_LABEL109	109
#define	IDC_LABEL110	110 // "Profit SUM"
#define	IDC_LABEL111	111
#define	IDC_LABEL112	112 // "Tax SUM"
#define	IDC_LABEL113	113
#define IDC_LISTBOX114	114 // Prompt Window

// ダイアログ IDD_DLGINIT
#define IDC_LABEL200	200
#define IDC_LABEL201	201
#define IDC_LABEL202	202
#define IDC_LABEL203	203
#define IDC_LABEL204	204
#define IDC_LABEL205	205
#define IDC_LABEL206	206
#define IDC_LABEL207	207
#define IDC_LABEL208	208
#define IDC_LABEL209	209
#define IDC_LABEL210	210
#define IDC_EDIT211		211 // [Tax]
#define IDC_EDIT212		212 // [Month1]
#define IDC_EDIT213		213 // [Day1]
#define IDC_EDIT214		214 // [Hour1]
#define IDC_EDIT215		215 // [Minute1]
#define IDC_EDIT216		216 // [Month2]
#define IDC_EDIT217		217 // [Day2]
#define IDC_EDIT218		218 // [Hour2]
#define IDC_EDIT219		219 // [Minute2]
#define IDC_EDIT220		220 // [Month3]
#define IDC_EDIT221		221 // [Day3]
#define IDC_EDIT222		222 // [Hour3]
#define IDC_EDIT223		223 // [Minute3]

// ダイアログ IDD_DLGVERSION
#define	IDC_STATIC300		300
#define	IDC_LABEL301		301
#define	IDC_LABEL302		302
#define	IDC_LABEL303		303
#define	IDC_LABEL304		304

//==========================================================
//  メニューリソース
//==========================================================
// メニュー IDM_FORM_MENU
#define IDM_OPTIMIZE	400
#define	IDM_IMPORT		401
#define	IDM_EXPORT		402
#define	IDM_EXIT		403
#define IDM_HELP		404
#define	IDM_VERSION		405

#endif