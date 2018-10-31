#include "cmnctrl.h"

//==========================================================
// LPCTSTR 用 strtok 関数改造版
//==========================================================
LPTSTR lptstrtok(LPCTSTR src, LPTSTR dest[], const TCHAR delim, int SplitNum, int* ElemNum)
{
	int i, j = 1, len;
	LPTSTR buff;

	if (src == NULL) {
		return NULL;
	} else {
		len = _tcslen(src);
		buff = (LPTSTR)malloc(sizeof(TCHAR) * (len + 1));
	}

	if (buff == NULL) {
		return NULL;
	} else {
		_tcscpy(buff, src);
		dest[0] = buff;
	}

	for (i=1; i<len; i++) {
		if (j == SplitNum) {
			break;
		}
		if (buff[i] == delim) {
			buff[i] = _T('\0');
			dest[j++] = (buff + i) + 1;
		}
	}
	dest[j] = NULL;	//番兵としてNULLを末尾に登録
	*ElemNum = j;

	return buff;
}

//==========================================================
// 文字列検査関数 ('0'～'9'のみかチェック)
//==========================================================
BOOL IsNumeric(LPCTSTR src)
{
	int i = 0;
	while (src[i] != NULL) {
		if (!_istdigit(src[i++])) return FALSE;
	}
	return TRUE;
}

//==========================================================
// ログ書き出し関数
//==========================================================
void WriteLog(HANDLE hFile, LPCTSTR Text)
{
	SYSTEMTIME StTime;
	TCHAR TextBuff[128], TimeStamp[100];
	DWORD dwWriten;

	GetLocalTime(&StTime);
	_stprintf(TimeStamp, _T("[%02d/%02d - %02d:%02d] "), StTime.wMonth, StTime.wDay, StTime.wHour, StTime.wMinute);

	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, TimeStamp, lstrlen(TimeStamp)*sizeof(TCHAR), &dwWriten, NULL);
	WriteFile(hFile, Text, lstrlen(Text)*sizeof(TCHAR), &dwWriten, NULL);
	WriteFile(hFile, _T("\r\n"), 2*sizeof(TCHAR), &dwWriten, NULL);
	FlushFileBuffers(hFile);
}

//==========================================================
// プロンプト制御関数
//==========================================================
void WritePrompt(HWND hDlg, LPCTSTR text)
{
	int ItemCount = SendDlgItemMessage(hDlg, IDC_LISTBOX114, LB_GETCOUNT, 0, 0);
	if (ItemCount == MAX_RESUME_PROMPT) {
		SendDlgItemMessage(hDlg, IDC_LISTBOX114, LB_DELETESTRING, 0, 0);
	}
	SendDlgItemMessage(hDlg, IDC_LISTBOX114, LB_ADDSTRING, 0, (LPARAM)text);
	SendDlgItemMessage(hDlg, IDC_LISTBOX114, LB_SETTOPINDEX, ItemCount-1, 0);
}

//==========================================================
// タイムフォーマットチェック関数
//==========================================================
BOOL TimeFormatCheck(int Year, int Month, int Day, int Hour, int Minit)
{
	BOOL Uruudoshi;

	if ((Month > 12) || (Hour > 23) || (Minit > 59)) return FALSE;

	if (Year % 400) {
		if (Year % 100) {
			if (Year % 4) {
				Uruudoshi = FALSE;
			} else {
				Uruudoshi = TRUE;
			}
		} else {
			Uruudoshi = FALSE;
		}
	} else {
		Uruudoshi = TRUE;
	}

	switch (Month) {
		case 0 : {
			return FALSE;
		}
		case 2 : {
			if (Uruudoshi) {
				if (Day > 29 || Day < 1) return FALSE;
			} else {
				if (Day > 28 || Day < 1) return FALSE;
			}
			break;
		}
		case 4 : case 6 : case 9 : case 11 : {
			if (Day > 30 || Day < 1) return FALSE;
			break;
		}
		default : {
			if (Day > 31 || Day < 1) return FALSE;
			break;
		}
	}

	return TRUE;
}

//==========================================================
// タイムフォーマット数値化関数
//==========================================================
double TimeToNum(int Month, int Day, int Hour, int Minit)
{
	return (Month + (((((Minit / 60.) + Hour) / 24.) + Day) / 31.));
}
