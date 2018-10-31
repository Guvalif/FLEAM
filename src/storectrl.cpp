#include "storectrl.h""

//==========================================================
// 各種内部関数プロトタイプ宣言
//==========================================================
static int round(int);
static int CulValue(int, int, BOOL);
static int FastStockInsert(FILE_INFO*, STOCK_INFO*, BOOL);


//==========================================================
// 商品売却 (@@SELS, @@SELD)
//==========================================================
BOOL SellStock(HWND hDlg, FILE_INFO* FileInfo, SETTING_INFO* SettingInfo, LPCTSTR Tag, int Quantity, int TimeNum, int ValueNum)
{
	sqlite3_stmt* stmt;
	int Value, Stock, Buy[3], Profit, Tax, TaxTemp = 0;
	LPTSTR SQLBuff;

	BOOL IsStockTable = TRUE;
	BOOL IsSELD = FALSE;
	int res;

	if (TimeNum != ValueNum) IsSELD = TRUE;	// @@SELDで呼び出されたと判定

	switch (ValueNum) {
		case 1 : {
			LPCTSTR SelectItemSQL = _T("SELECT Value1, Stock FROM ItemTable WHERE ItemTable.Tag = ?;");
			SQLBuff = (LPTSTR)SelectItemSQL;
			break;
		}
		case 2 : {
			LPCTSTR SelectItemSQL = _T("SELECT Value2, Stock FROM ItemTable WHERE ItemTable.Tag = ?;");
			SQLBuff = (LPTSTR)SelectItemSQL;
			break;
		}
		case 3 : {
			LPCTSTR SelectItemSQL = _T("SELECT Value3, Stock FROM ItemTable WHERE ItemTable.Tag = ?;");
			SQLBuff = (LPTSTR)SelectItemSQL;
			break;
		}
		default : {
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
			return FALSE;
		}
	}

	_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
	_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
	res = sqlite3_step(stmt);

	if (res != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 21 : Not Found Stock Info."));
		return FALSE;
	}

	// 価格と在庫数の取得
	Value = sqlite3_column_int(stmt, 0);
	Stock = sqlite3_column_int(stmt, 1);
	sqlite3_finalize(stmt);

	// インフィニティフラグのチェック
	if (Stock == -1) {
		switch (TimeNum) {
			case 1 : {
				LPCTSTR SelectStockSQL = _T("SELECT Buy1, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)SelectStockSQL;
				break;
			}
			case 2 : {
				LPCTSTR SelectStockSQL = _T("SELECT Buy2, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)SelectStockSQL;
				break;
			}
			case 3 : {
				LPCTSTR SelectStockSQL = _T("SELECT Buy3, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)SelectStockSQL;
				break;
			}
		}

		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
		res = sqlite3_step(stmt);

		// 販売済みかチェック
		if (res != SQLITE_ROW) {
			sqlite3_finalize(stmt);
			IsStockTable = FALSE;	// 販売データ登録済みフラグをOFF
			goto InsertStockTable;
		}

		// 販売数、純利益、税分の取得
		Buy[TimeNum-1] = sqlite3_column_int(stmt, 0);
		Profit = sqlite3_column_int(stmt, 1);
		Tax = sqlite3_column_int(stmt, 2);
		sqlite3_finalize(stmt);
	} else {
		LPCTSTR SelectStockSQL = _T("SELECT Buy1, Buy2, Buy3, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");

		_tsqlite3_prepare(FileInfo->DB, SelectStockSQL, -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
		res = sqlite3_step(stmt);

		// 販売済みかチェック
		if (res != SQLITE_ROW) {
			if (Stock < Quantity) {
				sqlite3_finalize(stmt);
				MessageBeep(-1);
				WritePrompt(hDlg, _T("$ Error 23 : Not Enough Stock."));
				return FALSE;
			}

			sqlite3_finalize(stmt);
			IsStockTable = FALSE;	// 販売データ登録済みフラグをOFF
			goto InsertStockTable;
		}

		// 各販売数の取得
		Buy[0] = sqlite3_column_int(stmt, 0);
		Buy[1] = sqlite3_column_int(stmt, 1);
		Buy[2] = sqlite3_column_int(stmt, 2);
		
		if (Stock < (Buy[0]+Buy[1]+Buy[2]+Quantity)) {
			sqlite3_finalize(stmt);
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 23 : Not Enough Stock."));
			return FALSE;
		}

		// 純利益、税分の取得
		Profit = sqlite3_column_int(stmt, 3);
		Tax = sqlite3_column_int(stmt, 4);
		sqlite3_finalize(stmt);
	}

	// 販売データの作成及び更新
	InsertStockTable:
	if (!IsStockTable) {
		switch (TimeNum) {
			case 1 : {
				LPCTSTR InsertStockSQL = _T("INSERT INTO StockTable(Tag, Buy1, Buy2, Buy3, Profit, Tax) VALUES(?, ?, 0, 0, ?, ?);");
				SQLBuff = (LPTSTR)InsertStockSQL;
				break;
			}
			case 2 : {
				LPCTSTR InsertStockSQL = _T("INSERT INTO StockTable(Tag, Buy1, Buy2, Buy3, Profit, Tax) VALUES(?, 0, ?, 0, ?, ?);");
				SQLBuff = (LPTSTR)InsertStockSQL;
				break;
			}
			case 3 : {
				LPCTSTR InsertStockSQL = _T("INSERT INTO StockTable(Tag, Buy1, Buy2, Buy3, Profit, Tax) VALUES(?, 0, 0, ?, ?, ?);");
				SQLBuff = (LPTSTR)InsertStockSQL;
				break;
			}
		}

		// 販売情報の書き込み
		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, Quantity);
		sqlite3_bind_int(stmt, 3, (Value*Quantity));
		if (!SettingInfo->AddTax) {
			sqlite3_bind_int(stmt, 4, TaxTemp);
		} else if (Value > DEF_TAX_MAX_PRICE) {
			sqlite3_bind_int(stmt, 4, (TaxTemp = TAX_MAX*Quantity));
		} else if (Value >= ADD_TAX_BORDER) {
			sqlite3_bind_int(stmt, 4, (TaxTemp = round((Value*SettingInfo->Tax/100)*Quantity)));
		} else {
			sqlite3_bind_int(stmt, 4, TaxTemp);
		}
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	} else {
		switch (TimeNum) {
			case 1 : {
				LPCTSTR UpdateStockSQL = _T("UPDATE StockTable SET Buy1 = ?, Profit = ?, Tax = ? WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)UpdateStockSQL;
				break;
			}
			case 2 : {
				LPCTSTR UpdateStockSQL = _T("UPDATE StockTable SET Buy2 = ?, Profit = ?, Tax = ? WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)UpdateStockSQL;
				break;
			}
			case 3 : {
				LPCTSTR UpdateStockSQL = _T("UPDATE StockTable SET Buy3 = ?, Profit = ?, Tax = ? WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)UpdateStockSQL;
				break;
			}
		}

		// 販売情報の書き込み
		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1, (Buy[TimeNum-1]+Quantity));
		sqlite3_bind_int(stmt, 2, (Profit+(Value*Quantity)));
		if (!SettingInfo->AddTax) {
			sqlite3_bind_int(stmt, 3, Tax);
		} else if (Value > DEF_TAX_MAX_PRICE) {
			sqlite3_bind_int(stmt, 3, (Tax + (TaxTemp = TAX_MAX*Quantity)));
		} else if (Value >= ADD_TAX_BORDER) {
			sqlite3_bind_int(stmt, 3, (Tax + (TaxTemp = round((Value*SettingInfo->Tax/100)*Quantity))));
		} else {
			sqlite3_bind_int(stmt, 3, Tax);
		}
		_tsqlite3_bind_text(stmt, 4, Tag, -1, SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}

	// 純利益、税分の加算
	SettingInfo->ProfitSUM[TimeNum-1] += (Value*Quantity);
	if (Value >= ADD_TAX_BORDER) {
		SettingInfo->TaxSUM[TimeNum-1] += TaxTemp;
	}

	// 純利益、税分の表示
	SetDlgItemInt(hDlg, IDC_LABEL110, (SettingInfo->ProfitSUM[0] + SettingInfo->ProfitSUM[1] + SettingInfo->ProfitSUM[2]), TRUE);
	SetDlgItemInt(hDlg, IDC_LABEL112, (SettingInfo->TaxSUM[0] + SettingInfo->TaxSUM[1] + SettingInfo->TaxSUM[2]), TRUE);

	// プロンプト、ログの書き込み
	if (!IsSELD) {
		TCHAR TextBuff[60];

		WritePrompt(hDlg, _T("$ Sell Stock :"));
		_stprintf(TextBuff, _T("%s * %d : [Use Value%d] Profit + \\%d, Tax + \\%d"), Tag, Quantity, ValueNum, (Value*Quantity), TaxTemp);
		WritePrompt(hDlg, TextBuff);
		WriteLog(FileInfo->SaleLog, TextBuff);
	} else {
		TCHAR TextBuff[128];

		WritePrompt(hDlg, _T("$ Sell Stock in Defined Value :"));
		_stprintf(TextBuff, _T("%s * %d : [Def Value%d] Profit + \\%d, Tax + \\%d"), Tag, Quantity, ValueNum, (Value*Quantity), TaxTemp);
		WritePrompt(hDlg, TextBuff);
		WriteLog(FileInfo->SaleLog, TextBuff);
	}
	
	return TRUE;
}

/* SellStock Sub Function */
static int round(int num)
{
	return ((num/10)*10);
}


//==========================================================
// 商品情報取得 (@@INFO)
//==========================================================
BOOL GetStockInfo(HWND hDlg, sqlite3* DB, LPCTSTR Tag)
{
	sqlite3_stmt* stmt;
	int Value[3], Stock, Buy[3], Profit, Tax;
	int res;

	LPCTSTR SelectItemSQL = _T("SELECT Value1, Value2, Value3, Stock FROM ItemTable WHERE ItemTable.Tag = ?;");

	_tsqlite3_prepare(DB, SelectItemSQL, -1, &stmt, NULL);
	_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
	res = sqlite3_step(stmt);

	if (res != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 21 : Not Found Stock Info."));
		return FALSE;
	} else {
		TCHAR TextBuff[60];
		LPCTSTR SelectStockSQL = _T("SELECT Buy1, Buy2, Buy3, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");

		// 各価格、在庫数の取得
		Value[0] = sqlite3_column_int(stmt, 0);
		Value[1] = sqlite3_column_int(stmt, 1);
		Value[2] = sqlite3_column_int(stmt, 2);
		Stock = sqlite3_column_int(stmt, 3);
		sqlite3_finalize(stmt);

		_tsqlite3_prepare(DB, SelectStockSQL, -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
		sqlite3_step(stmt);

		// 各販売数、純利益、税分の取得
		Buy[0] = sqlite3_column_int(stmt, 0);
		Buy[1] = sqlite3_column_int(stmt, 1);
		Buy[2] = sqlite3_column_int(stmt, 2);
		Profit = sqlite3_column_int(stmt, 3);
		Tax = sqlite3_column_int(stmt, 4);
		sqlite3_finalize(stmt);

		// プロンプトの書き込み
		WritePrompt(hDlg, _T("$ Get Stock Info :"));
		if (Stock != -1) {
			_stprintf(TextBuff, _T("%s : Values(\\%d, \\%d, \\%d) Stock %d/%d"), Tag, Value[0], Value[1], Value[2], Stock-Buy[0]-Buy[1]-Buy[2], Stock);
		} else {
			_stprintf(TextBuff, _T("%s : Values(\\%d, \\%d, \\%d) Stock ∞"), Tag, Value[0], Value[1], Value[2]);
		}
		WritePrompt(hDlg, TextBuff);
		_stprintf(TextBuff, _T("     : Sold(%d, %d, %d) Profit \\%d Tax \\%d"), Buy[0], Buy[1], Buy[2], Profit, Tax);
		WritePrompt(hDlg, TextBuff);
	}

	return TRUE;
}

//==========================================================
// 商品情報登録 (@@SETS)
//==========================================================
BOOL SetStockInfo(HWND hDlg, FILE_INFO* FileInfo, STOCK_INFO* StockInfo)
{
	sqlite3_stmt* stmt;
	int res;

	LPCTSTR SelectItemSQL = _T("SELECT * FROM ItemTable WHERE ItemTable.Tag = ?;");

	_tsqlite3_prepare(FileInfo->DB, SelectItemSQL, -1, &stmt, NULL);
	_tsqlite3_bind_text(stmt, 1, StockInfo->Tag, -1, SQLITE_STATIC);
	res = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	if (res == SQLITE_ROW) {
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 22 : Same Tag is Used."));
		return FALSE;
	} else {
		TCHAR TextBuff[128];
		LPCTSTR InsertItemSQL =_T("INSERT INTO ItemTable(Tag, Value1, Value2, Value3, Stock) VALUES(?, ?, ?, ?, ?);");

		// 商品情報の登録
		_tsqlite3_prepare(FileInfo->DB, InsertItemSQL, -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, StockInfo->Tag, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, StockInfo->Value[0]);
		sqlite3_bind_int(stmt, 3, StockInfo->Value[1]);
		sqlite3_bind_int(stmt, 4, StockInfo->Value[2]);
		sqlite3_bind_int(stmt, 5, StockInfo->Stock);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		// プロンプト、ログの書き込み
		WritePrompt(hDlg, _T("$ Set Stock Info :"));
		if (StockInfo->Stock != -1) {
			_stprintf(TextBuff, _T("%s : Values(\\%d, \\%d, \\%d) Stock %d"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2], StockInfo->Stock);
			WritePrompt(hDlg, TextBuff);
			_stprintf(TextBuff, _T("[@@SETS] : [%s] Values = (%d, %d, %d) JPY, Stock = %d"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2], StockInfo->Stock);
		} else {
			_stprintf(TextBuff, _T("%s : Values(\\%d, \\%d, \\%d) Stock ∞"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2]);
			WritePrompt(hDlg, TextBuff);
			_stprintf(TextBuff, _T("[@@SETS] : [%s] Values = (%d, %d, %d) JPY, Stock = ∞"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2]);
		}
		WriteLog(FileInfo->SystemLog, TextBuff);
	}

	// オートコンプリート機能の実行 (TAG = "CNNN" という形式の時のみ)
	if (IsNumeric((StockInfo->Tag)+1)) {
		TCHAR Buff[20];

		_stprintf(Buff, _T("@@sets %c%03d "), StockInfo->Tag[0], _ttoi((StockInfo->Tag)+1)+1);
		SetDlgItemText(hDlg, IDC_EDIT102, Buff);
		SendDlgItemMessage(hDlg, IDC_EDIT102, EM_SETSEL, 12, 12);

		return FALSE;	//空白描画抑止 (失敗ではない)
	}

	return TRUE;
}


//==========================================================
// 商品情報更新 (@@UPDS)
//==========================================================
BOOL UpdateStockInfo(HWND hDlg, FILE_INFO* FileInfo, STOCK_INFO* StockInfo)
{
	sqlite3_stmt* stmt;
	int res;

	LPCTSTR SelectItemSQL = _T("SELECT * FROM ItemTable WHERE ItemTable.Tag = ?;");

	_tsqlite3_prepare(FileInfo->DB, SelectItemSQL, -1, &stmt, NULL);
	_tsqlite3_bind_text(stmt, 1, StockInfo->Tag, -1, SQLITE_STATIC);
	res = sqlite3_step(stmt);

	if (res != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 21 : Not Found Stock Info."));
		return FALSE;
	} else {
		int Buy[3] = { 0 };
		TCHAR TextBuff[128];

		LPCTSTR SQL[] = {
			_T("SELECT Buy1, Buy2, Buy3 FROM StockTable WHERE StockTable.Tag = ?;"),
			_T("UPDATE ItemTable SET Value1 = ?, Value2 = ?, Value3 = ?, Stock = ? WHERE ItemTable.Tag = ?;")
		};

		sqlite3_finalize(stmt);
		_tsqlite3_prepare(FileInfo->DB, SQL[0], -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, StockInfo->Tag, -1, SQLITE_STATIC);
		res = sqlite3_step(stmt);

		// 販売数と新たな在庫数の整合性チェック
		if (res == SQLITE_ROW) {
			Buy[0] = sqlite3_column_int(stmt, 0);
			Buy[1] = sqlite3_column_int(stmt, 1);
			Buy[2] = sqlite3_column_int(stmt, 2);
			if ((StockInfo->Stock < (Buy[0]+Buy[1]+Buy[2])) && (StockInfo->Stock != -1)) {
				sqlite3_finalize(stmt);
				MessageBeep(-1);
				WritePrompt(hDlg, _T("$ Error 23 : Not Enough Stock."));
				return FALSE;
			}
		}
		sqlite3_finalize(stmt);

		// 商品情報の更新
		_tsqlite3_prepare(FileInfo->DB, SQL[1], -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1, StockInfo->Value[0]);
		sqlite3_bind_int(stmt, 2, StockInfo->Value[1]);
		sqlite3_bind_int(stmt, 3, StockInfo->Value[2]);
		sqlite3_bind_int(stmt, 4, StockInfo->Stock);
		_tsqlite3_bind_text(stmt, 5, StockInfo->Tag, -1, SQLITE_STATIC);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		// プロンプト。ログの書き込み
		WritePrompt(hDlg, _T("$ Update Stock Info :"));
		if (StockInfo->Stock != -1) {
			_stprintf(TextBuff, _T("%s : NewValues(\\%d, \\%d, \\%d) NewStock %d"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2], StockInfo->Stock);
			WritePrompt(hDlg, TextBuff);
			_stprintf(TextBuff, _T("[@@UPDS] : [%s] NewValues = (%d, %d, %d) JPY, NewStock = %d"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2], StockInfo->Stock);
		} else {
			_stprintf(TextBuff, _T("%s : NewValues(\\%d, \\%d, \\%d) NewStock ∞"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2]);
			WritePrompt(hDlg, TextBuff);
			_stprintf(TextBuff, _T("[@@UPDS] : [%s] NewValues = (%d, %d, %d) JPY, NewStock = ∞"), StockInfo->Tag, StockInfo->Value[0], StockInfo->Value[1], StockInfo->Value[2]);
		}
		WriteLog(FileInfo->SystemLog, TextBuff);
	}

	return TRUE;
}


//==========================================================
// 商品返品 (@@SRES)
//==========================================================
BOOL ReshopStock(HWND hDlg, FILE_INFO* FileInfo, SETTING_INFO* SettingInfo, LPCTSTR Tag, int TimeNum, int Value)
{
	sqlite3_stmt* stmt;
	int Buy, Profit, ValueTemp, Tax[2];
	LPTSTR SQLBuff;

	int res;

	switch (TimeNum) {
		case 1 : {
			LPCTSTR SelectStockSQL = _T("SELECT Buy1, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");
			SQLBuff = (LPTSTR)SelectStockSQL;
			break;
		}
		case 2 : {
			LPCTSTR SelectStockSQL = _T("SELECT Buy2, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");
			SQLBuff = (LPTSTR)SelectStockSQL;
			break;
		}
		case 3 : {
			LPCTSTR SelectStockSQL = _T("SELECT Buy3, Profit, Tax FROM StockTable WHERE StockTable.Tag = ?;");
			SQLBuff = (LPTSTR)SelectStockSQL;
			break;
		}
		default : {
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 12 : Bad Argment."));
			return FALSE;
		}
	}

	_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
	_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
	res = sqlite3_step(stmt);

	if (res != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 25 : Not Sold."));
		return FALSE;
	} else {
		TCHAR TextBuff[128];

		switch (TimeNum) {
			case 1 : {
				LPCTSTR UpdateStockSQL = _T("UPDATE StockTable SET Buy1 = ?, Profit = ?, Tax = ? WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)UpdateStockSQL;
				break;
			}
			case 2 : {
				LPCTSTR UpdateStockSQL = _T("UPDATE StockTable SET Buy2 = ?, Profit = ?, Tax = ? WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)UpdateStockSQL;
				break;
			}
			case 3 : {
				LPCTSTR UpdateStockSQL = _T("UPDATE StockTable SET Buy3 = ?, Profit = ?, Tax = ? WHERE StockTable.Tag = ?;");
				SQLBuff = (LPTSTR)UpdateStockSQL;
				break;
			}
		}

		// 販売情報の取得
		Buy = sqlite3_column_int(stmt, 0);
		Profit = sqlite3_column_int(stmt, 1);
		Tax[0] = sqlite3_column_int(stmt, 2);
		sqlite3_finalize(stmt);

		// 売却確認
		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
		if (Buy) {
			sqlite3_bind_int(stmt, 1, (Buy - 1));
		} else {
			sqlite3_finalize(stmt);
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 26 : Not Sold in that Time."));
			return FALSE;
		}

		ValueTemp = Value;
		Value = CulValue(Value, SettingInfo->Tax, SettingInfo->AddTax);
		Tax[1] = ValueTemp-Value;

		// 返金額の整合性チェック
		if (Profit >= Value) {
			sqlite3_bind_int(stmt, 2, (Profit - Value));
		} else {
			sqlite3_finalize(stmt);
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 24 : Not Enough Profit."));
			return FALSE;
		}

		if (Tax[0] >= Tax[1]) {
			sqlite3_bind_int(stmt, 3, (Tax[0] - Tax[1]));
		} else {
			sqlite3_finalize(stmt);
			MessageBeep(-1);
			WritePrompt(hDlg, _T("$ Error 24 : Not Enough Profit."));
			return FALSE;
		}

		// 返金処理
		_tsqlite3_bind_text(stmt, 4, Tag, -1, SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		// 純利益、税分の減算
		SettingInfo->ProfitSUM[TimeNum-1] -= Value;
		if (Tax[1]) {
			SettingInfo->TaxSUM[TimeNum-1] -= Tax[1];
		}

		// 純利益、税分の表示
		SetDlgItemInt(hDlg, IDC_LABEL110, (SettingInfo->ProfitSUM[0] + SettingInfo->ProfitSUM[1] + SettingInfo->ProfitSUM[2]), TRUE);
		SetDlgItemInt(hDlg, IDC_LABEL112, (SettingInfo->TaxSUM[0] + SettingInfo->TaxSUM[1] + SettingInfo->TaxSUM[2]), TRUE);

		// プロンプト、ログの書き込み
		_stprintf(TextBuff, _T("$ Reshop Stock : [%s] Profit - \\%d, Tax - \\%d"), Tag, Value, Tax[1]);
		WritePrompt(hDlg, TextBuff);
		_stprintf(TextBuff, _T("[@@RESS] : [%s] Profit - \\%d, Tax - \\%d"), Tag, Value, Tax[1]);
		WriteLog(FileInfo->SystemLog, TextBuff);
	}

	return TRUE;
}

/* StockReshop Sub Function */
static int CulValue(int Value, int Tax, BOOL AddTax)
{
	if (!AddTax) {
		return Value;
	}
	else if (((Value*100)/(100+Tax)) > DEF_TAX_MAX_PRICE) {
		return (Value - TAX_MAX);
	}
	else if ((Value*100)%(100+Tax)) {
		Value += ((Value*100)%(100+Tax))/10;
	}
	return ((Value*100)/(100+Tax));
}


//==========================================================
// 商品削除 (@@SDEL)
//==========================================================
BOOL DeleteStock(HWND hDlg, FILE_INFO* FileInfo, LPCTSTR Tag)
{
	sqlite3_stmt* stmt;
	int res;

	LPCTSTR SelectItemSQL = _T("SELECT * FROM ItemTable WHERE ItemTable.Tag = ?;");

	_tsqlite3_prepare(FileInfo->DB, SelectItemSQL, -1, &stmt, NULL);
	_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
	res = sqlite3_step(stmt);

	if (res != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Error 21 : Not Found Stock Info."));
		return FALSE;
	} else {
		TCHAR TextBuff[60];
		LPCTSTR DeleteItemSQL = _T("DELETE FROM ItemTable WHERE ItemTable.Tag = ?;");
		LPCTSTR DeleteStockSQL = _T("DELETE FROM StockTable WHERE StockTable.Tag = ?;");

		// 商品情報の削除
		_tsqlite3_prepare(FileInfo->DB, DeleteItemSQL, -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		// 販売情報の削除
		_tsqlite3_prepare(FileInfo->DB, DeleteStockSQL, -1, &stmt, NULL);
		_tsqlite3_bind_text(stmt, 1, Tag, -1, SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		// プロンプト、ログの書き込み
		_stprintf(TextBuff, _T("$ Delete Stock : %s is Deleted."), Tag);
		WritePrompt(hDlg, TextBuff);
		_stprintf(TextBuff, _T("[@@DELS] : [%s] is Deleted"), Tag);
		WriteLog(FileInfo->SystemLog, TextBuff);
	}

	return TRUE;
}


//==========================================================
// 商品リスト読込 (@@IMPL)
//==========================================================
BOOL ImportItemList(HWND hDlg, FILE_INFO* FileInfo)
{
	FILE* fp;
	LPTSTR Text;
	int counter = 0, setnum;

	fp = _tfopen(_T("ItemList.csv"), _T("r"));
	
	if (fp == NULL) {
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Import Item List : Failed."));
		return FALSE;
	} else {
		TCHAR TextBuff[128], TagBuff[5];
		STOCK_INFO StockInfo;
		BOOL bLast, Arg = TRUE;
		int len, i = 0;

		CsvConstructor(fp);	//CsvReaderの初期化

		// *.csv の解析及び商品情報の登録
		sqlite3_exec(FileInfo->DB, "BEGIN", NULL, NULL, NULL);
		while ((len = CsvGetElement(&bLast, TextBuff, 128)) >= 0) {
			switch (i) {
				case 0 : {
					if (len == 4) {
						lstrcpyn(TagBuff, TextBuff, 5);
						StockInfo.Tag = TagBuff;
					} else {
						Arg = FALSE;
					}
					break;
				}
				case 1 : {
					if (IsNumeric(TextBuff)) {
						StockInfo.Value[0] = _ttoi(TextBuff);
					} else {
						Arg = FALSE;
					}
					break;
				}
				case 2 : {
					if (IsNumeric(TextBuff)) {
						StockInfo.Value[1] = _ttoi(TextBuff);
					} else {
						Arg = FALSE;
					}
					break;
				}
				case 3 : {
					if (IsNumeric(TextBuff)) {
						StockInfo.Value[2] = _ttoi(TextBuff);
					} else {
						Arg = FALSE;
					}
					break;
				}
				case 4 : {
					if (IsNumeric(TextBuff) || (_ttoi(TextBuff) == -1)) {
						StockInfo.Stock = _ttoi(TextBuff);
					} else {
						Arg = FALSE;
					}
					break;
				}
				default : {
					Arg = FALSE;
				}
			}

			i++;
			if (bLast) {
				if ((i == 5) && Arg) {
					FastStockInsert(FileInfo, &StockInfo, FALSE);
				}
				i = 0;	Arg = TRUE;
				counter++;
			}
		}
		Text = TextBuff;
	}
	sqlite3_exec(FileInfo->DB, "COMMIT", NULL, NULL, NULL);
	setnum = FastStockInsert(NULL, NULL, TRUE);
	fclose(fp);

	// プロンプト、ログの書き込み
	_stprintf(Text, _T("$ Import Item List : Success. (%d/%d)"), setnum, counter);
	WritePrompt(hDlg, Text);
	_stprintf(Text, _T("[@@IMPL] : %d Items are Imported. (Number of All Items is %d)"), setnum, counter);
	WriteLog(FileInfo->SystemLog, Text);

	return TRUE;
}

/* ImportItemList Sub Function */
static int FastStockInsert(FILE_INFO* FileInfo, STOCK_INFO* StockInfo, BOOL EndFlag)
{
	static BOOL Init = TRUE;
	static sqlite3_stmt *stmt_select, *stmt_insert;
	static counter;
	int res;

	// 終了時処理 (初期化)
	if (EndFlag) {
		int temp = counter;

		Init = TRUE;
		counter = 0;
		sqlite3_finalize(stmt_select);
		sqlite3_finalize(stmt_insert);
		return temp;
	}

	// 初回実行時の処理
	if (Init) {
		LPCTSTR SQL[] = {
			_T("SELECT * FROM ItemTable WHERE ItemTable.Tag = ?;"),
			_T("INSERT INTO ItemTable(Tag, Value1, Value2, Value3, Stock) VALUES(?, ?, ?, ?, ?);")
		};
		_tsqlite3_prepare(FileInfo->DB, SQL[0], -1, &stmt_select, NULL);
		_tsqlite3_prepare(FileInfo->DB, SQL[1], -1, &stmt_insert, NULL);
		Init = FALSE;
	}

	// 商品情報の登録
	_tsqlite3_bind_text(stmt_select, 1, StockInfo->Tag, -1, SQLITE_STATIC);
	res = sqlite3_step(stmt_select);

	if (res == SQLITE_ROW) {
		sqlite3_clear_bindings(stmt_select);
		sqlite3_reset(stmt_select);
		return -1;
	} else {
		_tsqlite3_bind_text(stmt_insert, 1, StockInfo->Tag, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt_insert, 2, StockInfo->Value[0]);
		sqlite3_bind_int(stmt_insert, 3, StockInfo->Value[1]);
		sqlite3_bind_int(stmt_insert, 4, StockInfo->Value[2]);
		sqlite3_bind_int(stmt_insert, 5, StockInfo->Stock);

		sqlite3_step(stmt_insert);
		counter++;
	}

	// 引数の破棄
	sqlite3_clear_bindings(stmt_select);
	sqlite3_reset(stmt_select);
	sqlite3_clear_bindings(stmt_insert);
	sqlite3_reset(stmt_insert);
	return 0;
}


//==========================================================
// 商品リスト書出し (@@EXPL)
//==========================================================
BOOL ExportItemList(HWND hDlg, FILE_INFO* FileInfo)
{
	FILE* fp;
	DWORD dwWriten;
	sqlite3_stmt* stmt;

	fp = _tfopen(_T("ItemList.csv"), _T("w"));
	if (fp == NULL) {
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Export Item List : Failed."));
		return FALSE;
	}
	_tsqlite3_prepare(FileInfo->DB, _T("SELECT replace(Tag, '\"', '\"\"'), Value1, Value2, Value3, Stock FROM ItemTable ORDER BY Tag ASC;"), -1, &stmt, NULL);

	// ヘッダーの書き込み
	_fputts(_T("Tag,Value1,Value2,Value3,Quantity\n"), fp);
	_fputts(_T("--------,--------,--------,--------,--------\n"), fp);

	// 商品情報の書き込み
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		static TCHAR TextBuff[256];

		_stprintf(TextBuff, _T("\"%s\",%d,%d,%d,%d\n"),
			_tsqlite3_column_text(stmt, 0),
			sqlite3_column_int(stmt, 1),
			sqlite3_column_int(stmt, 2),
			sqlite3_column_int(stmt, 3),
			sqlite3_column_int(stmt, 4)
			);

		_fputts(TextBuff, fp);
	}
	fclose(fp);

	// プロンプト、ログの書き込み
	WritePrompt(hDlg, _T("$ Export Item List : Success."));
	WriteLog(FileInfo->SystemLog, _T("[@@EXPL] : Exporting Item List is Success."));
	
	return TRUE;
}


//==========================================================
// レシート印字 (@@PRNT)
//==========================================================
BOOL PrintReceipt(HWND hDlg, FILE_INFO* FileInfo, SETTING_INFO* SettingInfo, SYSTEMTIME* StTime, LPCTSTR Mode)
{
	HANDLE hFile;
	DWORD dwWriten;

	sqlite3_stmt* stmt;
	TCHAR TextBuff[128];
	LPTSTR SQLBuff;
	int Sum[2];
	static int TransNum;

	_stprintf(TextBuff, _T("./Receipt[%04d].txt"), TransNum);
	hFile = CreateFile(TextBuff, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBeep(-1);
		WritePrompt(hDlg, _T("$ Print Receipt : Failed."));
		return FALSE;
	}

	// ヘッダーの書き込み
	GetLocalTime(StTime);
	_stprintf(TextBuff, _T("Time : %02d/%02d - %02d:%02d\r\n"), StTime->wMonth, StTime->wDay, StTime->wHour, StTime->wMinute);
	WriteFile(hFile, TextBuff, lstrlen(TextBuff)*sizeof(TCHAR), &dwWriten, NULL);
	_stprintf(TextBuff, _T("TransNo. : %04d\r\n"), TransNum);
	WriteFile(hFile, TextBuff, lstrlen(TextBuff)*sizeof(TCHAR), &dwWriten, NULL);
	WriteFile(hFile, _T("------------------------------------------------------------\r\n"), 62*sizeof(TCHAR), &dwWriten, NULL);

	if (_tcsicmp(Mode, _T("CLOSE")) == 0) {
		double NowTime;
		NowTime = TimeToNum(StTime->wMonth, StTime->wDay, StTime->wHour, StTime->wMinute);

		if (NowTime <= SettingInfo->TimeNum[0]) {
			LPCTSTR SelectStockSQL = _T("SELECT Tag, buy1 FROM StockTable ORDER BY Tag ASC;");
			SQLBuff = (LPTSTR)SelectStockSQL;
			Sum[0] = SettingInfo->ProfitSUM[0];
			Sum[1] = SettingInfo->TaxSUM[0];
		} else if (NowTime <= SettingInfo->TimeNum[1]) {
			LPCTSTR SelectStockSQL = _T("SELECT Tag, buy2 FROM StockTable WHERE buy2 > 0 ORDER BY Tag ASC;");
			SQLBuff = (LPTSTR)SelectStockSQL;
			Sum[0] = SettingInfo->ProfitSUM[1];
			Sum[1] = SettingInfo->TaxSUM[1];
		} else if (NowTime <= SettingInfo->TimeNum[2]) {
			LPCTSTR SelectStockSQL = _T("SELECT Tag, buy3 FROM StockTable WHERE buy3 > 0 ORDER BY Tag ASC;");
			SQLBuff = (LPTSTR)SelectStockSQL;
			Sum[0] = SettingInfo->ProfitSUM[2];
			Sum[1] = SettingInfo->TaxSUM[2];
		} else {
			LPCTSTR SelectStockSQL = _T("SELECT * FROM StockTable ORDER BY Tag ASC;");

			_tsqlite3_prepare(FileInfo->DB, SelectStockSQL, -1, &stmt, NULL);

			// 販売情報の書き込み
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				_stprintf(TextBuff, _T("[%s] : Sold(%d, %d, %d), Profit \\%d, Tax \\%d\r\n"),
					_tsqlite3_column_text(stmt, 0),
					sqlite3_column_int(stmt, 1),
					sqlite3_column_int(stmt, 2),
					sqlite3_column_int(stmt, 3),
					sqlite3_column_int(stmt, 4),
					sqlite3_column_int(stmt, 5)
					);

				WriteFile(hFile, TextBuff, lstrlen(TextBuff)*sizeof(TCHAR), &dwWriten, NULL);
			}
			sqlite3_finalize(stmt);

			WriteFile(hFile, _T("------------------------------------------------------------\r\n"), 62*sizeof(TCHAR), &dwWriten, NULL);

			Sum[0] = SettingInfo->ProfitSUM[0] + SettingInfo->ProfitSUM[1] + SettingInfo->ProfitSUM[2];
			Sum[1] = SettingInfo->TaxSUM[0] + SettingInfo->TaxSUM[1] + SettingInfo->TaxSUM[2];

			goto End;
		}

		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);

		// 販売情報の書き込み
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			_stprintf(TextBuff, _T("[%s] : Sold %d\r\n"),
				_tsqlite3_column_text(stmt, 0),
				sqlite3_column_int(stmt, 1)
				);

			WriteFile(hFile, TextBuff, lstrlen(TextBuff)*sizeof(TCHAR), &dwWriten, NULL);
		}
		sqlite3_finalize(stmt);

		WriteFile(hFile, _T("------------------------------------------------------------\r\n"), 62*sizeof(TCHAR), &dwWriten, NULL);
	} else {
		switch (Mode[0]) {
			case '*' : {
				LPCTSTR SelectStockSQL = _T("SELECT * FROM StockTable WHERE Tag GLOB '[*]*' ORDER BY Tag ASC;");
				SQLBuff = (LPTSTR)SelectStockSQL;
				break;
			}
			case '?' : {
				LPCTSTR SelectStockSQL = _T("SELECT * FROM StockTable WHERE Tag GLOB '[?]*' ORDER BY Tag ASC;");
				SQLBuff = (LPTSTR)SelectStockSQL;
				break;
			}
			case '\'' : {
				LPCTSTR SelectStockSQL = _T("SELECT * FROM StockTable WHERE Tag GLOB '['']*' ORDER BY Tag ASC;");
				SQLBuff = (LPTSTR)SelectStockSQL;
				break;
			}
			default : {
				_stprintf(TextBuff, _T("SELECT * FROM StockTable WHERE Tag GLOB '[%s]*' ORDER By Tag ASC;"), Mode);
				SQLBuff = (LPTSTR)TextBuff;
			}
		}

		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);

		// 販売情報の書き込み
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			_stprintf(TextBuff, _T("[%s] : Sold(%d, %d, %d), Profit \\%d, Tax \\%d\r\n"),
				_tsqlite3_column_text(stmt, 0),
				sqlite3_column_int(stmt, 1),
				sqlite3_column_int(stmt, 2),
				sqlite3_column_int(stmt, 3),
				sqlite3_column_int(stmt, 4),
				sqlite3_column_int(stmt, 5)
				);

			WriteFile(hFile, TextBuff, lstrlen(TextBuff)*sizeof(TCHAR), &dwWriten, NULL);
		}
		sqlite3_finalize(stmt);

		WriteFile(hFile, _T("------------------------------------------------------------\r\n"), 62*sizeof(TCHAR), &dwWriten, NULL);

		switch (Mode[0]) {
			case '*' : {
				LPCTSTR SumProfitSQL = _T("SELECT sum(Profit) FROM StockTable WHERE Tag GLOB '[*]*';");
				SQLBuff = (LPTSTR)SumProfitSQL;
				break;
			}
			case '?' : {
				LPCTSTR SumProfitSQL = _T("SELECT sum(Profit) FROM StockTable WHERE Tag GLOB '[?]*';");
				SQLBuff = (LPTSTR)SumProfitSQL;
				break;
			}
			case '\'' : {
				LPCTSTR SumProfitSQL = _T("SELECT sum(Profit) FROM StockTable WHERE Tag GLOB '['']*';");
				SQLBuff = (LPTSTR)SumProfitSQL;
				break;
			}
			default : {
				_stprintf(TextBuff, _T("SELECT sum(Profit) FROM StockTable WHERE Tag GLOB '[%s]*';"), Mode);
				SQLBuff = (LPTSTR)TextBuff;
			}
		}

		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
		sqlite3_step(stmt);
		Sum[0] = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);

		switch (Mode[0]) {
			case '\'' : {
				LPCTSTR SumTaxSQL = _T("SELECT sum(Tax) FROM StockTable WHERE Tag GLOB '['']*';");
				SQLBuff = (LPTSTR)SumTaxSQL;
				break;
			}
			default : {
				_stprintf(TextBuff, _T("SELECT sum(Tax) FROM StockTable WHERE Tag GLOB '[%s]*';"), Mode);
				SQLBuff = (LPTSTR)TextBuff;
			}
		}

		_tsqlite3_prepare(FileInfo->DB, SQLBuff, -1, &stmt, NULL);
		sqlite3_step(stmt);
		Sum[1] = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
	}
	End:	//純利益、税分の書き込み
	_stprintf(TextBuff, _T("[SUM] : Profit %d, Tax %d\r\n"), Sum[0], Sum[1]);
	WriteFile(hFile, TextBuff, lstrlen(TextBuff)*sizeof(TCHAR), &dwWriten, NULL);
	_stprintf(TextBuff, _T("$ Print Receipt : Mode = %s Success."), Mode);
	WritePrompt(hDlg, TextBuff);

	if (++TransNum == 10000) TransNum = 0;	//トランザクションナンバーのロール

	CloseHandle(hFile);
	return TRUE;
}
