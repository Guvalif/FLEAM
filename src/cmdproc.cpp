#include "cmdproc.h"

//==========================================================
// コマンドプロシージャ
//==========================================================
BOOL CmdProc(
	HWND hDlg,
	FLAG_INFO* FlagInfo, FILE_INFO* FileInfo, SETTING_INFO* SettingInfo, SYSTEMTIME* StTime,
	LPCTSTR cmd
	)
{
	int elem_n, res;
	LPTSTR elem[SPLIT_MAX+1], buff;

	const TCHAR delim = _T(' ');	// スペースを区切り文字と設定

	buff = lptstrtok(cmd, elem, delim, SPLIT_MAX, &elem_n);

	// メモリ不足の検出
	if (buff == NULL) {
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 04 : Allocate Memory is Failed."));
		return FALSE;
	}

	// 末尾の空白の検出
	if (_tcscmp(elem[elem_n-1], _T("")) == 0) {
		WritePrompt(hDlg, _T("$ Error 13 : White Space is in End of Command Line."));
		free(buff);
		return FALSE;
	}

	if (lstrlen(elem[0]) == COMMAND_LEN) {
		/* デバッグ用コマンド */
		if (_tcsicmp(elem[0], _T("@@dbug")) == 0) {
			if (!FlagInfo->Mgr) {
				MessageBeep(-1);
				WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
				free(buff);
				return FALSE;
			}
			switch (_ttoi(elem[1])) {
				case 0 : {
					sqlite3_exec(FileInfo->DB, elem[2], NULL, NULL, NULL);
					WritePrompt(hDlg, _T("$ Execute SQL"));
					break;
				}

				case 1 : {
					if (SettingInfo->AddTax) {
						SettingInfo->AddTax = FALSE;
						WritePrompt(hDlg, _T("$ SettingInfo.AddTax = FALSE"));
					} else {
						SettingInfo->AddTax = TRUE;
						WritePrompt(hDlg, _T("$ SettingInfo.AddTax = TRUE"));
					}
					break;
				}
				
				default : {
					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error : Undefined Mode."));
				}
			}

			free(buff);
			return TRUE;
		}
		switch (elem_n) {
			case 1 : {
				/* 優先処理コマンド */
				// "@@MGRM" 管理者権限取得 (into Manager Mode)
				if (_tcsicmp(elem[0], _T("@@mgrm")) == 0) {
					if (FlagInfo->Mgr) {
						FlagInfo->Mgr = FALSE;
						SetWindowText(hDlg, APP_TITLE);
						WritePrompt(hDlg, _T("$ Escape from Manager Mode."));
						break;
					}
					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 06 : Done Escape from Manager Mode."));
					free(buff);
					return FALSE;
				}

				/* 管理者権限要求コマンド */
				// "@@IMPL" 商品リスト読込 (Import Item List)
				else if (_tcsicmp(elem[0], _T("@@impl")) == 0) {
					if (!FlagInfo->Mgr) {
						MessageBeep(-1);
						WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
						free(buff);
						return FALSE;
					}
					res = ImportItemList(hDlg, FileInfo);
					free(buff);
					return res;
				}

				/* 汎用コマンド */
				// "@@EXPL" 商品リスト書出し (Export Item List)
				else if (_tcsicmp(elem[0], _T("@@expl")) == 0) {
					res = ExportItemList(hDlg, FileInfo);
					free(buff);
					return res;
				}
				// "@@VSET" セッティング参照 (View Settings)
				else if (_tcsicmp(elem[0], _T("@@vset")) == 0) {
					TCHAR TextBuff[60];
					WritePrompt(hDlg, _T("$ View Settings :"));
					_stprintf(TextBuff, _T("Tax : %d %%"), SettingInfo->Tax);
					WritePrompt(hDlg, TextBuff);
					_stprintf(TextBuff, _T("Time1 : %02d/%02d - %02d:%02d"), SettingInfo->Time[0].Month, SettingInfo->Time[0].Day, SettingInfo->Time[0].Hour, SettingInfo->Time[0].Minute);
					WritePrompt(hDlg, TextBuff);
					_stprintf(TextBuff, _T("Time2 : %02d/%02d - %02d:%02d"), SettingInfo->Time[1].Month, SettingInfo->Time[1].Day, SettingInfo->Time[1].Hour, SettingInfo->Time[1].Minute);
					WritePrompt(hDlg, TextBuff);
					_stprintf(TextBuff, _T("Time3 : %02d/%02d - %02d:%02d"), SettingInfo->Time[2].Month, SettingInfo->Time[2].Day, SettingInfo->Time[2].Hour, SettingInfo->Time[2].Minute);
					WritePrompt(hDlg, TextBuff);
					break;
				}
				// "@@VSAL" 販売ログ参照 (View Sale Log)
				else if (_tcsicmp(elem[0], _T("@@vsal")) == 0) {
					TCHAR TextBuff[128];
					_stprintf(TextBuff, _T("./Store%d/SaleLog%d.log"), StTime->wYear, StTime->wYear);
					ShellExecute(NULL, _T("open"), _T("notepad.exe"), TextBuff, NULL, SW_SHOWNORMAL);
					break;
				}
				// "@@VSYS" システムログ参照 (View System Log)
				else if (_tcsicmp(elem[0], _T("@@vsys")) == 0) {
					TCHAR TextBuff[128];
					_stprintf(TextBuff, _T("./Store%d/SystemLog%d.log"), StTime->wYear, StTime->wYear);
					ShellExecute(NULL, _T("open"), _T("notepad.exe"), TextBuff, NULL, SW_SHOWNORMAL);
					break;
				}

				goto command_undefined;
			}

			case 2 : {
				/* 優先処理コマンド */
				// "@@MGRM" 管理者権限取得 (into Manager Mode)
				if (_tcsicmp(elem[0], _T("@@mgrm")) == 0) {
					if (_tcscmp(elem[1], _T("nnct8192")) == 0) {
						if (!FlagInfo->Mgr) {
							FlagInfo->Mgr = TRUE;
							SetWindowText(hDlg, APP_TITLE_MGR);
							WritePrompt(hDlg, _T("$ Into Manager Mode."));
							break;
						}

						MessageBeep(-1);
						WritePrompt(hDlg, _T("$ Error 05 : Done into Manager Mode."));
						free(buff);
						return FALSE;
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 03 : Wrong Password."));
					free(buff);
					return FALSE;
				}

				/* 管理者権限要求コマンド */
				// "@@DELS" 商品削除 (Delete Stock)
				else if (_tcsicmp(elem[0], _T("@@dels")) == 0) {
					if (!FlagInfo->Mgr) {
						MessageBeep(-1);
						WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
						free(buff);
						return FALSE;
					}
					if (lstrlen(elem[1]) == 4) {
						res = DeleteStock(hDlg, FileInfo, elem[1]);
						free(buff);
						return res;
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
					free(buff);
					return FALSE;
				}

				/* 汎用コマンド */
				// "@@INFO" 商品情報取得 (Get Stock Information)
				else if (_tcsicmp(elem[0], _T("@@info")) == 0) {
					if (lstrlen(elem[1]) == 4) {
						res = GetStockInfo(hDlg, FileInfo->DB, elem[1]);
						free(buff);
						return res;
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
					free(buff);
					return FALSE;
				}
				// "@@PRNT" レシート印字 (Print Receipt)
				else if (_tcsicmp(elem[0], _T("@@prnt")) == 0) {
					if (lstrlen(elem[1]) == 1
						|| _tcsicmp(elem[1], _T("close")) == 0
						)
					{
						res = PrintReceipt(hDlg, FileInfo, SettingInfo, StTime, elem[1]);
						free(buff);
						return res;
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
					free(buff);
					return res;
				}

				goto command_undefined;
			}

			case 3 : {
				/* 汎用コマンド */
				// "@@SELS" 商品販売 (Sell Stock)
				if (_tcsicmp(elem[0], _T("@@sels")) == 0) {
					if ((lstrlen(elem[1]) == 4)
						&& IsNumeric(elem[2])
						)
					{
						double NowTime;
						GetLocalTime(StTime);
						NowTime = TimeToNum(StTime->wMonth, StTime->wDay, StTime->wHour, StTime->wMinute);

						if (NowTime <= SettingInfo->TimeNum[0]) {
							res = SellStock(hDlg, FileInfo, SettingInfo, elem[1], _ttoi(elem[2]), 1, 1);
							free(buff);
							return res;
						} else if (NowTime <= SettingInfo->TimeNum[1]) {
							res = SellStock(hDlg, FileInfo, SettingInfo, elem[1], _ttoi(elem[2]), 2, 2);
							free(buff);
							return res;
						} else if (NowTime <= SettingInfo->TimeNum[2]) {
							res = SellStock(hDlg, FileInfo, SettingInfo, elem[1], _ttoi(elem[2]), 3, 3);
							free(buff);
							return res;
						} else {
							MessageBeep(-1);
							WritePrompt(hDlg, _T("$ Error 02 : Time Over."));
							free(buff);
							return FALSE;
						}
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
					free(buff);
					return FALSE;
				}

				goto command_undefined;
			}

			case 4 : {
				/* 管理者権限要求コマンド */
				// "@@SELD" 価格指定売却 (Sell Stock in Defined Value)
				if (_tcsicmp(elem[0], _T("@@seld")) == 0) {
					if (!FlagInfo->Mgr) {
						MessageBeep(-1);
						WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
						free(buff);
						return FALSE;
					}
					if ((lstrlen(elem[1]) == 4)
						&& IsNumeric(elem[2])
						&& IsNumeric(elem[3])
						)
					{
						double NowTime;
						GetLocalTime(StTime);
						NowTime = TimeToNum(StTime->wMonth, StTime->wDay, StTime->wHour, StTime->wMinute);

						if (NowTime <= SettingInfo->TimeNum[0]) {
							res = SellStock(hDlg, FileInfo, SettingInfo, elem[1], _ttoi(elem[2]), 1, _ttoi(elem[3]));
							free(buff);
							return res;
						} else if (NowTime <= SettingInfo->TimeNum[1]) {
							res = SellStock(hDlg, FileInfo, SettingInfo, elem[1], _ttoi(elem[2]), 2, _ttoi(elem[3]));
							free(buff);
							return res;
						} else if (NowTime <= SettingInfo->TimeNum[2]) {
							res = SellStock(hDlg, FileInfo, SettingInfo, elem[1], _ttoi(elem[2]), 3, _ttoi(elem[3]));
							free(buff);
							return res;
						} else {
							MessageBeep(-1);
							WritePrompt(hDlg, _T("$ Error 02 : Time Over."));
							free(buff);
							return FALSE;
						}
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
					free(buff);
					return FALSE;
				}
				// "@@RESS" 商品返品 (Reshop Stock)
				else if (_tcsicmp(elem[0], _T("@@ress")) == 0) {
					if (!FlagInfo->Mgr) {
						MessageBeep(-1);
						WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
						free(buff);
						return FALSE;
					}
					if ((lstrlen(elem[1]) == 4)
						&& IsNumeric(elem[2])
						&& IsNumeric(elem[3])
						)
					{
						res = ReshopStock(hDlg, FileInfo, SettingInfo, elem[1], _ttoi(elem[2]), _ttoi(elem[3]));
						free(buff);
						return res;
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment"));
					free(buff);
					return FALSE;
				}

				goto command_undefined;
			}

			case 6 : {
				/* 管理者権限要求コマンド */
				// "@@SETS" 商品情報登録 (Set Stock Information)
				if (_tcsicmp(elem[0], _T("@@sets")) == 0) {
					if (!FlagInfo->Mgr) {
						MessageBeep(-1);
						WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
						free(buff);
						return FALSE;
					}
					if ((lstrlen(elem[1]) == 4)
						&& IsNumeric(elem[2])
						&& IsNumeric(elem[3])
						&& IsNumeric(elem[4])
						&& (IsNumeric(elem[5]) || (_ttoi(elem[5]) == -1))
						)
					{
						STOCK_INFO StockInfo;
						StockInfo.Tag = elem[1];
						StockInfo.Value[0] = _ttoi(elem[2]);
						StockInfo.Value[1] = _ttoi(elem[3]);
						StockInfo.Value[2] = _ttoi(elem[4]);
						StockInfo.Stock = _ttoi(elem[5]);

						res = SetStockInfo(hDlg, FileInfo, &StockInfo);
						free(buff);
						return res;
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
					free(buff);
					return FALSE;
				}
				// "@@UPDS" 商品情報更新 (Update Stock Information)
				else if (_tcsicmp(elem[0], _T("@@upds")) == 0) {
					if (!FlagInfo->Mgr) {
						MessageBeep(-1);
						WritePrompt(hDlg, _T("$ Error 01 : Not Allowed. (Please into Manager Mode.)"));
						free(buff);
						return FALSE;
					}
					if ((lstrlen(elem[1]) == 4)
						&& IsNumeric(elem[2])
						&& IsNumeric(elem[3])
						&& IsNumeric(elem[4])
						&& (IsNumeric(elem[5]) || (_ttoi(elem[5]) == -1))
						)
					{
						STOCK_INFO StockInfo;
						StockInfo.Tag = elem[1];
						StockInfo.Value[0] = _ttoi(elem[2]);
						StockInfo.Value[1] = _ttoi(elem[3]);
						StockInfo.Value[2] = _ttoi(elem[4]);
						StockInfo.Stock = _ttoi(elem[5]);

						res = UpdateStockInfo(hDlg, FileInfo, &StockInfo);
						free(buff);
						return res;
					}

					MessageBeep(-1);
					WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
					free(buff);
					return FALSE;
				}

				goto command_undefined;
			}

			default : {
				goto command_undefined;
			}
		}
	} else {
		command_undefined:	//構文及びコマンド未定義時の処理

		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 11 : Undefined Syntax. (Command Line)"));
		free(buff);
		return FALSE;
	}

	free(buff);
	return TRUE;
}