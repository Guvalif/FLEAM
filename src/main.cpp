//==========================================================
// FLE@M ver.1.07
// Function of Labor-saving Equipment at Marketing.
// Copyright (C) 2010-2011 by Kazuyuki Takase.
// ---------------------------------------------------------
// 学園祭フリーマーケット運営支援ツール
//==========================================================
#pragma comment(lib, "sqlite3_bcc.lib")
#pragma comment(lib, "shlwapi.lib")

#define STRICT
#define WIN32_LEAN_AND_MEAN


/* Windows32 API Header */
#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <shlwapi.h>

/* Standard API Header */
#include <stdio.h>
#include <stdlib.h>

/* Other API & Resource Header */
#include "cmdproc.h"
#include "cmnctrl.h"
#include "resource.h"
#include "storectrl.h"
#include "tsqlite3.h"
#include "type.h"

//==========================================================
// 各種関数プロトタイプ宣言
//==========================================================
/* プロシージャ */
BOOL CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK VersionDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK InitDlgProc(HWND, UINT, WPARAM, LPARAM);

/* ハンドラ */
BOOL Push_IDC_BUTTON100(HWND);
BOOL Push_IDC_BUTTON101(HWND);

/* システム処理 */
BOOL InitApp(HINSTANCE);
void ExitApp(void);

/* コールバック関数 */
int setting_callback(void*, int, char**, char**);

//==========================================================
// 各種グローバル変数 (*_INFO型はtype.h参照)
//==========================================================
static FLAG_INFO FlagInfo;			// 各種フラグ情報
static FILE_INFO FileInfo;			// 各種ファイルハンドル
static SETTING_INFO SettingInfo;	// 各種設定情報
static SYSTEMTIME StTime;			// 時間取得用


//==========================================================
// アプリケーションエントリーポイント
//==========================================================
#ifdef UNICODE
extern "C"
#endif

int WINAPI _tWinMain(
	HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow
	)
{
	HWND hDlg;
	HACCEL hAccel;
	MSG Msg;

	if (!InitApp(hInstance)) {
		MessageBox(NULL, _T("初期化に失敗しました。"), APP_NAME, MB_APPLMODAL | MB_ICONERROR);
		return -1;
	}

	if (FlagInfo.Init) {
		// メインウィンドウ生成
		hDlg = CreateDialog(hInstance, _T("IDD_DLGMAIN"), NULL, MainDlgProc);

		if (hDlg == NULL) {
			MessageBox(NULL, _T("ウィンドウ生成に失敗しました。"), APP_NAME, MB_APPLMODAL | MB_ICONERROR);
			return -1;
		}

		// キーボードアクセラレータの読込
		hAccel = LoadAccelerators(hInstance, _T("IDA_MAIN"));

		while (GetMessage(&Msg, NULL, 0, 0)) {
			if (!TranslateAccelerator(hDlg, hAccel, &Msg) &&
				!IsDialogMessage(hDlg, &Msg)
				)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}

	ExitApp();
	return 0;
}

//==========================================================
// ダイアログプロシージャ (IDD_DLGMAIN)
//==========================================================
BOOL CALLBACK MainDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG : {
			TCHAR TextBuff[60];

			// アイコンの読込
			HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE);
			HICON hIcon = (HICON)LoadImage(hInstance, _T("IDI_ICON"), IMAGE_ICON, 16, 16, 0);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			// SettingInfoの表示
			WritePrompt(hDlg, _T("$ Load Settings :"));
			_stprintf(TextBuff, _T("Tax : %d %%"), SettingInfo.Tax);
			WritePrompt(hDlg, TextBuff);
			_stprintf(TextBuff, _T("Time1 : %02d/%02d - %02d:%02d"), SettingInfo.Time[0].Month, SettingInfo.Time[0].Day, SettingInfo.Time[0].Hour, SettingInfo.Time[0].Minute);
			WritePrompt(hDlg, TextBuff);
			_stprintf(TextBuff, _T("Time2 : %02d/%02d - %02d:%02d"), SettingInfo.Time[1].Month, SettingInfo.Time[1].Day, SettingInfo.Time[1].Hour, SettingInfo.Time[1].Minute);
			WritePrompt(hDlg, TextBuff);
			_stprintf(TextBuff, _T("Time3 : %02d/%02d - %02d:%02d"), SettingInfo.Time[2].Month, SettingInfo.Time[2].Day, SettingInfo.Time[2].Hour, SettingInfo.Time[2].Minute);
			WritePrompt(hDlg, TextBuff);
			WritePrompt(hDlg, _T("$ Initialize Application is Success."));

			// ProfitSUMとTaxSUMの表示
			SetDlgItemInt(hDlg, IDC_LABEL110, (SettingInfo.ProfitSUM[0] + SettingInfo.ProfitSUM[1] + SettingInfo.ProfitSUM[2]), TRUE);
			SetDlgItemInt(hDlg, IDC_LABEL112, (SettingInfo.TaxSUM[0] + SettingInfo.TaxSUM[1] + SettingInfo.TaxSUM[2]), TRUE);

			FlagInfo.Mgr = TRUE;
			SettingInfo.AddTax = TRUE;
			break;
		}

		case WM_COMMAND : {
			switch (LOWORD(wParam)) {
				// SELS ボタン押下時処理
				case IDC_BUTTON100 : {
					Push_IDC_BUTTON100(hDlg);
					break;
				}

				// INFO ボタン押下時処理
				case IDC_BUTTON101 : {
					Push_IDC_BUTTON101(hDlg);
					break;
				}

				// Optimize DB 押下時処理
				case IDM_OPTIMIZE : {
					sqlite3_exec(FileInfo.DB, "VACUUM", NULL, NULL, NULL);
					WritePrompt(hDlg, _T("$ Optimize DataBase : Success."));
					break;
				}

				// Import ItemList 押下時処理
				case IDM_IMPORT : {
					if (FlagInfo.Mgr) {
						ImportItemList(hDlg, &FileInfo);
						break;
					}
					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
					break;
				}

				// Export ItemList 押下時処理
				case IDM_EXPORT : {
					ExportItemList(hDlg, &FileInfo);
					break;
				}

				// Command Reference 押下時処理
				case IDM_HELP : {
					if (PathFileExists(_T("Command Reference.txt"))) {
						ShellExecute(NULL, _T("open"), _T("notepad.exe"), _T("Command Reference.txt"), NULL, SW_SHOWNORMAL);
					} else {
						MessageBox(hDlg, _T("\"Command Reference.txt\"が存在しません。"), APP_NAME, MB_APPLMODAL | MB_ICONERROR);
					}
					break;
				}

				// Version Information 押下時処理
				case IDM_VERSION : {
					HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE);
					DialogBox(hInstance, _T("IDD_DLGVERSION"), hDlg, VersionDlgProc);
					break;
				}

				// Exit 押下時処理
				case IDM_EXIT : {
					if (FlagInfo.Mgr) {
						DestroyWindow(hDlg);
						break;
					}
					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
					break;
				}

				// Enter Key 押下時処理
				case IDOK : {
					HWND Focus = GetFocus();	//キーフォーカスの取得

					// フォーカスが[Command Line]に存在
					if (Focus == GetDlgItem(hDlg, IDC_EDIT102)) {
						TCHAR CmdLineBuff[COMMAND_LINE_MAX];
						BOOL res;

						if (GetDlgItemText(hDlg, IDC_EDIT102, CmdLineBuff, COMMAND_LINE_MAX)) {
							res = CmdProc(hDlg, &FlagInfo, &FileInfo, &SettingInfo, &StTime, CmdLineBuff);
							if (res) {
								SetDlgItemText(hDlg, IDC_EDIT102, _T(""));
							}
						}
						break;
					}
					// フォーカスが[Quantity]に存在
					else if (Focus == GetDlgItem(hDlg, IDC_EDIT104)) {
						Push_IDC_BUTTON100(hDlg);
						break;
					}
					// フォーカスが[Tag]に存在
					else if (Focus == GetDlgItem(hDlg, IDC_EDIT103)) {
						Push_IDC_BUTTON101(hDlg);
						break;
					}
					break;
				}

				// ESC Key 押下時処理
				case IDCANCEL : {
					if (FlagInfo.Mgr) {
						FlagInfo.Mgr = FALSE;
						SetWindowText(hDlg, APP_TITLE);
						WritePrompt(hDlg, _T("$ Escape from Manager Mode."));
					}
					break;
				}

				default : {
					break;
				}
			}
			break;
		}

		case WM_CLOSE : {
			if (FlagInfo.Mgr) {
				DestroyWindow(hDlg);
				break;
			}
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
			break;
		}

		case WM_DESTROY : {
			PostQuitMessage(0);
			break;
		}

		default : {
			break;
		}
	}

	return FALSE;
}

//==========================================================
// ボタン押下ハンドラ (IDC_BUTTON100 [SELS])
//==========================================================
BOOL Push_IDC_BUTTON100(HWND hDlg)
{
	TCHAR TagBuff[5];
	int Quantity;	BOOL res;

	if (GetDlgItemText(hDlg, IDC_EDIT103, TagBuff, 5)) {
		if (lstrlen(TagBuff) != 4) {
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 14 : Undefined Tag is Inputed."));
			SetFocus(GetDlgItem(hDlg, IDC_EDIT103));
			return FALSE;
		}

		Quantity = GetDlgItemInt(hDlg, IDC_EDIT104, &res, FALSE);
		if (res) {
			double NowTime;
			GetLocalTime(&StTime);
			NowTime = TimeToNum(StTime.wMonth, StTime.wDay, StTime.wHour, StTime.wMinute);

			// 現在時刻による引数決定
			if (NowTime <= SettingInfo.TimeNum[0]) {
				res = SellStock(hDlg, &FileInfo, &SettingInfo, TagBuff, Quantity, 1, 1);
			}
			else if (NowTime <= SettingInfo.TimeNum[1]) {
				res = SellStock(hDlg, &FileInfo, &SettingInfo, TagBuff, Quantity, 2, 2);
			}
			else if (NowTime <= SettingInfo.TimeNum[2]) {
				res = SellStock(hDlg, &FileInfo, &SettingInfo, TagBuff, Quantity, 3, 3);
			}
			else {
				MessageBeep(-1);
				WritePrompt(hDlg, _T("$ Error 02 : Time Over."));
			}

			// EditBoxのクリア
			if (res) {
				SetDlgItemText(hDlg, IDC_EDIT104, _T(""));
				SetDlgItemText(hDlg, IDC_EDIT103, _T(""));
			}

			SetFocus(GetDlgItem(hDlg, IDC_EDIT103));
			return TRUE;
		} else {
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 16 : Please Input Quantity."));
			SetFocus(GetDlgItem(hDlg, IDC_EDIT104));
			return FALSE;
		}
	}

	GetDlgItemInt(hDlg, IDC_EDIT104, &res, FALSE);
	if (res) {
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 15 : Please Input Tag."));
		SetFocus(GetDlgItem(hDlg, IDC_EDIT103));
	}

	return FALSE;
}

//==========================================================
// ボタン押下ハンドラ (IDC_BUTTON101 [INFO])
//==========================================================
BOOL Push_IDC_BUTTON101(HWND hDlg)
{
	TCHAR TagBuff[5];

	if (GetDlgItemText(hDlg, IDC_EDIT103, TagBuff, 5)) {
		if (lstrlen(TagBuff) != 4) {
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 14 : Undefined Tag is Inputed."));
			return FALSE;
		}

		if (GetStockInfo(hDlg, FileInfo.DB, TagBuff)) {
			SetDlgItemText(hDlg, IDC_EDIT103, _T(""));
		}
		return TRUE;
	}

	return FALSE;
}

//==========================================================
// ダイアログプロシージャ (IDD_DLGVERSION)
//==========================================================
BOOL CALLBACK VersionDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_COMMAND : {
			switch (LOWORD(wParam)) {
				case IDOK : {
				}
				case IDCANCEL : {
					EndDialog(hDlg, 0);
					return TRUE;
				}
				default : {
					break;
				}
			}
			break;
		}
		case WM_CLOSE : {
			EndDialog(hDlg, 0);
			return TRUE;
		}
		default : {
			break;
		}
	}

	return FALSE;
}

//==========================================================
// 初期化処理
//==========================================================
BOOL InitApp(HINSTANCE hInstance)
{
	TCHAR TextBuff[128];
	int res, SettingState;

	GetLocalTime(&StTime);

	// StoreYYYY ディレクトリの作成
	_stprintf(TextBuff, _T("./Store%d"), StTime.wYear);
	CreateDirectory(TextBuff, NULL);

	// ItemListYYYY.db のオープン(作成)
	_stprintf(TextBuff, _T("./Store%d/ItemList%d.db"), StTime.wYear, StTime.wYear);
	res = _tsqlite3_open(TextBuff, &FileInfo.DB);

	if (res != SQLITE_OK) {
		return FALSE;
	}

	// SettingYYYY.dat のオープン(作成)
	_stprintf(TextBuff, _T("./Store%d/Setting%d.dat"), StTime.wYear, StTime.wYear);
	res = _tsqlite3_open(TextBuff, &FileInfo.Setting);

	if (res != SQLITE_OK) {
		sqlite3_close(FileInfo.DB);
		return FALSE;
	}

	// SystemLogYYYY.log のオープン(作成)
	_stprintf(TextBuff, _T("./Store%d/SystemLog%d.log"), StTime.wYear, StTime.wYear);
	FileInfo.SystemLog = CreateFile(TextBuff, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (FileInfo.SystemLog == INVALID_HANDLE_VALUE) {
		sqlite3_close(FileInfo.DB);
		sqlite3_close(FileInfo.Setting);
		return FALSE;
	}

	// SaleLogYYYY.log のオープン(作成)
	_stprintf(TextBuff, _T("./Store%d/SaleLog%d.log"), StTime.wYear, StTime.wYear);
	FileInfo.SaleLog = CreateFile(TextBuff, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (FileInfo.SaleLog == INVALID_HANDLE_VALUE) {
		sqlite3_close(FileInfo.DB);
		sqlite3_close(FileInfo.Setting);
		CloseHandle(FileInfo.SystemLog);
		return FALSE;
	}

	// SettingYYYY.dat から初期化フラグを取得 (SettingState に初期化フラグを代入)
	res = sqlite3_exec(FileInfo.Setting, "SELECT elem1 FROM SettingTable WHERE SettingTable.opt = 'Init';", setting_callback, &SettingState, NULL);

	if (res != SQLITE_OK) {
		int i = 0;
		const char* InitSettingSQL[] = {
			"CREATE TABLE SettingTable(opt TEXT, elem1 INTEGER, elem2 INTEGER, elem3 INTEGER, elem4 INTEGER);",
			"INSERT INTO SettingTable(opt, elem1) VALUES('Init', 0);",
			"INSERT INTO SettingTable(opt) VALUES('Tax');",
			"INSERT INTO SettingTable(opt) VALUES('Time1');",
			"INSERT INTO SettingTable(opt) VALUES('Time2');",
			"INSERT INTO SettingTable(opt) VALUES('Time3');",
			"INSERT INTO SettingTable(opt, elem1, elem2) VALUES('Profit1', 0, 0);",
			"INSERT INTO SettingTable(opt, elem1, elem2) VALUES('Profit2', 0, 0);",
			"INSERT INTO SettingTable(opt, elem1, elem2) VALUES('Profit3', 0, 0);",
			NULL
		};

		const char* InitItemListSQL[] = {
			"CREATE TABLE ItemTable(Tag TEXT, Value1 INTEGER, Value2 INTEGER, Value3 INTEGER, Stock INTEGER);",
			"CREATE TABLE StockTable(Tag TEXT, Buy1 INTEGER, Buy2 INTEGER, Buy3 INTEGER, Profit INTEGER, Tax INTEGER);",
			NULL
		};

		// SettingYYYY.dat に InitSettingSQL を実行
		sqlite3_exec(FileInfo.Setting, "BEGIN", NULL, NULL, NULL);
		while (InitSettingSQL[i] != NULL) {
			sqlite3_exec(FileInfo.Setting, InitSettingSQL[i++], NULL, NULL, NULL);
		}
		sqlite3_exec(FileInfo.Setting, "COMMIT", NULL, NULL, NULL);

		// ItemListYYYY.db に InitItemListSQL を実行
		i = 0;
		while (InitItemListSQL[i] != NULL) {
			sqlite3_exec(FileInfo.DB, InitItemListSQL[i++], NULL, NULL, NULL);
		}

		// 初期設定用ダイアログをオープン
		DialogBox(hInstance, _T("IDD_DLGINIT"), NULL, InitDlgProc);
	} else {
		if (SettingState) {
			sqlite3_stmt* stmt;
			int i = 0;

			const char* RoadSettingSQL[] = {
				"SELECT elem1 FROM SettingTable WHERE SettingTable.opt = 'Tax';",
				"SELECT elem1, elem2, elem3, elem4 FROM SettingTable WHERE SettingTable.opt LIKE 'time_';",
				"SELECT elem1, elem2 FROM SettingTable WHERE SettingTable.opt LIKE 'profit_';"
			};

			// SettingInfo の初期化 (SettingYYYY.dat から設定読込)
			sqlite3_exec(FileInfo.Setting, RoadSettingSQL[0], setting_callback, &(SettingInfo.Tax), NULL);

			sqlite3_prepare(FileInfo.Setting, RoadSettingSQL[1], -1, &stmt, NULL);
			while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
				SettingInfo.Time[i].Month	= sqlite3_column_int(stmt, 0);
				SettingInfo.Time[i].Day		= sqlite3_column_int(stmt, 1);
				SettingInfo.Time[i].Hour	= sqlite3_column_int(stmt, 2);
				SettingInfo.Time[i].Minute	= sqlite3_column_int(stmt, 3);
				SettingInfo.TimeNum[i] = TimeToNum(SettingInfo.Time[i].Month, SettingInfo.Time[i].Day, SettingInfo.Time[i].Hour, SettingInfo.Time[i].Minute);
				i++;
			}
			sqlite3_finalize(stmt);

			i = 0;
			sqlite3_prepare(FileInfo.Setting, RoadSettingSQL[2], -1, &stmt, NULL);
			while ((res = sqlite3_step(stmt)) == SQLITE_ROW) {
				SettingInfo.ProfitSUM[i]	= sqlite3_column_int(stmt, 0);
				SettingInfo.TaxSUM[i]		= sqlite3_column_int(stmt, 1);
				i++;
			}
			sqlite3_finalize(stmt);

			FlagInfo.Init = TRUE;	//初期化完了フラグをON
			WriteLog(FileInfo.SystemLog, _T("Initialize Application is Success."));
			return TRUE;
		}
		// 初期設定用ダイアログをオープン
		DialogBox(hInstance, _T("IDD_DLGINIT"), NULL, InitDlgProc);
	}

	WriteLog(FileInfo.SystemLog, _T("Initialize Application is Success."));
	return TRUE;
}

//==========================================================
// sqlite3_exec コールバック関数 (セッティング取得用)
//==========================================================
int setting_callback(void* argc, int columns, char** value, char** name)
{
	int i;
	for (i=0; i<columns; i++) {
		*(((int*)argc)+i) = atoi(value[i]);
	}
	return 0;
}

//==========================================================
// ダイアログプロシージャ (IDD_DLGINIT)
//==========================================================
BOOL CALLBACK InitDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG : {
			// アイコンの読込
			HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE);
			HICON hIcon = (HICON)LoadImage(hInstance, _T("IDI_ICON"), IMAGE_ICON, 16, 16, 0);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
		case WM_COMMAND : {
			switch (LOWORD(wParam)) {
				case IDOK : {
					BOOL res;
					int i, EditBox[13];	// EditBoxの数と同じだけ確保

					// 全設定項目を取得 (IDはresource.h参照)
					for (i = IDC_EDIT211; i <= IDC_EDIT223; i++) {
						EditBox[i-IDC_EDIT211] = GetDlgItemInt(hDlg, i, &res, FALSE);
						if(!res) {
							MessageBox(hDlg, _T("全ての項目が入力されていません。"), APP_NAME, MB_APPLMODAL | MB_ICONERROR);
							return FALSE;
						}
					}

					// タイムフォーマットのチェック
					i = 3;
					while (i--) {
						if (!TimeFormatCheck(StTime.wYear, EditBox[1+(i*4)], EditBox[2+(i*4)], EditBox[3+(i*4)], EditBox[4+(i*4)])) {
							MessageBox(hDlg, _T("無効なタイムフォーマットが含まれています。"), APP_NAME, MB_APPLMODAL | MB_ICONERROR);
							return FALSE;
						}
					}

					// Time1 < Time2 < Time3 という順序になってるかチェック
					i = 3;
					while (i--) {
						SettingInfo.TimeNum[i] = TimeToNum(EditBox[1+(i*4)], EditBox[2+(i*4)], EditBox[3+(i*4)], EditBox[4+(i*4)]);
					}
					if ((SettingInfo.TimeNum[0] >= SettingInfo.TimeNum[1]) || (SettingInfo.TimeNum[1] >= SettingInfo.TimeNum[2])) {
						MessageBox(hDlg, _T("Time1 < Time2 < Time3 という順序を満たしていません。"), APP_NAME, MB_APPLMODAL | MB_ICONERROR);
						return FALSE;
					}

					LPCTSTR WriteSettingSQL[] = {
						_T("UPDATE SettingTable SET elem1 = ? WHERE SettingTable.opt = 'Tax';"),
						_T("UPDATE SettingTable SET elem1 = ?, elem2 = ?, elem3 = ?, elem4 = ? WHERE SettingTable.opt = ?;")
					};
					TCHAR TextBuff[10];
					sqlite3_stmt* stmt;

					// SettingYYYY.dat への書き込み
					sqlite3_exec(FileInfo.Setting, "BEGIN", NULL, NULL, NULL);
					_tsqlite3_prepare(FileInfo.Setting, WriteSettingSQL[1], -1, &stmt, NULL);
					for (i=0; i<3; i++) {
						sqlite3_bind_int(stmt, 1, EditBox[1+(i*4)]);
						sqlite3_bind_int(stmt, 2, EditBox[2+(i*4)]);
						sqlite3_bind_int(stmt, 3, EditBox[3+(i*4)]);
						sqlite3_bind_int(stmt, 4, EditBox[4+(i*4)]);
						_stprintf(TextBuff, _T("Time%d"), i+1);
						_tsqlite3_bind_text(stmt, 5, TextBuff, -1, SQLITE_STATIC);
						sqlite3_step(stmt);
						sqlite3_clear_bindings(stmt);
						sqlite3_reset(stmt);
					}
					sqlite3_finalize(stmt);
					sqlite3_exec(FileInfo.Setting, "COMMIT", NULL, NULL, NULL);

					_tsqlite3_prepare(FileInfo.Setting, WriteSettingSQL[0], -1, &stmt, NULL);
					sqlite3_bind_int(stmt, 1, EditBox[0]);
					sqlite3_step(stmt);
					sqlite3_finalize(stmt);

					sqlite3_exec(FileInfo.Setting, "UPDATE SettingTable SET elem1 = 1 WHERE SettingTable.opt = 'Init';", NULL, NULL, NULL);

					// SettingInfo の初期化
					SettingInfo.Tax = EditBox[0];
					i = 3;
					while (i--) {
						SettingInfo.Time[i].Month 	= EditBox[1+(i*4)];
						SettingInfo.Time[i].Day		= EditBox[2+(i*4)];
						SettingInfo.Time[i].Hour 	= EditBox[3+(i*4)];
						SettingInfo.Time[i].Minute 	= EditBox[4+(i*4)];
					}

					FlagInfo.Init = TRUE;	// 初期化完了フラグをON
					EndDialog(hDlg, 0);
					return TRUE;
				}
				case IDCANCEL : {
					EndDialog(hDlg, 0);
					return TRUE;
				}
				default : {
					break;
				}
			}
			break;
		}
		case WM_CLOSE : {
			EndDialog(hDlg, 0);
			return TRUE;
		}
		default : {
			break;
		}
	}

	return FALSE;
}

//==========================================================
// 終了時処理
//==========================================================
void ExitApp()
{
	int i, res;
	char Buff[10];
	sqlite3_stmt* stmt;

	const char* SaveSettingSQL = "UPDATE SettingTable SET elem1 = ?, elem2 = ? WHERE SettingTable.opt = ?;";

	// SettingYYYY.dat への書き込み
	sqlite3_prepare(FileInfo.Setting, SaveSettingSQL, -1, &stmt, NULL);
	for (i=0; i<3; i++) {
		sqlite3_bind_int(stmt, 1, SettingInfo.ProfitSUM[i]);
		sqlite3_bind_int(stmt, 2, SettingInfo.TaxSUM[i]);
		sprintf(Buff, "Profit%d", i+1);
		sqlite3_bind_text(stmt, 3, Buff, -1, SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_clear_bindings(stmt);
		sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);

	WriteLog(FileInfo.SystemLog, _T("Exit Application."));

	// ファイルハンドルのクローズ
	sqlite3_close(FileInfo.DB);
	sqlite3_close(FileInfo.Setting);
	CloseHandle(FileInfo.SystemLog);
	CloseHandle(FileInfo.SaleLog);
}