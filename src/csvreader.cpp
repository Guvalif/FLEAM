#include "csvreader.h"

//==========================================================
// 内部マクロ
//==========================================================
/* 要素数取得 */
#define dim(x) (sizeof(x)/sizeof((x)[0]))

/* ステートメントの定義 */
typedef enum {
	INIT = 0,
	COMMA = 1,
	CRLF = 2,
	NORMAL_DATA = 3,
	QUOTE_DATA = 4,
	ESCAPING = 5,
	UNQUOTE = 6,
} csvState;

//==========================================================
// 各種内部関数プロトタイプ宣言
//==========================================================
static int csvReadFile(void);
static BOOL csvGetNextChar(LPTSTR);
static void csvParser(TCHAR);

//==========================================================
// 各種内部変数
//==========================================================
static FILE* fp;
static TCHAR buff[BUFF_SIZE];
static long lIndex;
static csvState stmt;


//==========================================================
// コンストラクタ (初期化関数)
//==========================================================
void CsvConstructor(FILE* InFp)
{
	fp = InFp;
	memset(buff, 0x00, sizeof(buff));
	lIndex = 0;
	stmt = COMMA;
}


//==========================================================
// CSV読込関数
//==========================================================
int CsvGetElement(LPBOOL lpbLastElement, LPTSTR lpszElement, DWORD maxElementSize)
{
	TCHAR c;
	DWORD dwIndex = 0;
	BOOL bResult;

	*lpbLastElement = FALSE;
	stmt = INIT;

	while (bResult = csvGetNextChar(&c)) {
		csvParser(c);

		if (stmt == COMMA) {
			break;
		}
		else if (stmt == CRLF) {
			*lpbLastElement = TRUE;
			break;
		}
		else if (stmt == NORMAL_DATA || stmt == QUOTE_DATA) {
			if (dwIndex >= maxElementSize-1) {
				if (dwIndex > maxElementSize-1) {
					bResult = FALSE;
					break;
				}
			} else {
				lpszElement[dwIndex] = c;
				dwIndex++;
			}
		}
	}

	lpszElement[dwIndex] = _T('\0');

	return (bResult)?dwIndex:-1;
}


//==========================================================
// 内部関数 (読込部分)
//==========================================================
static int csvReadFile()
{
	if (_fgetts(buff, dim(buff), fp) == NULL) {
		return -1;
	}
	if (feof(fp)) {
		return -1;
	}
	return 1;
}


//==========================================================
// 内部関数 (判定部分)
//==========================================================
static BOOL csvGetNextChar(LPTSTR lpC)
{
	if (buff[lIndex] == _T('\0')) {
		lIndex = 0;
		if (csvReadFile() < 0) {
			return FALSE;
		}
	}

	*lpC = buff[lIndex];
	lIndex++;

	return TRUE;
}


//==========================================================
// 内部関数 (オートマトン)
//==========================================================
static void csvParser(TCHAR c)
{
	int iChar;
	static const csvState StateTable[][4] = {
	//  ','          '"'          '\r'or'\n'   etc
		COMMA,       UNQUOTE,     CRLF,        NORMAL_DATA,	// INIT
		INIT,        INIT,        INIT,        INIT,		// COMMA
		INIT,        INIT,        INIT,        INIT,		// CRLF
		COMMA,       NORMAL_DATA, CRLF,        NORMAL_DATA,	// NORMAL_DATA
		QUOTE_DATA,  ESCAPING,    QUOTE_DATA,  QUOTE_DATA,	// QUOTE_DATA
		COMMA,       QUOTE_DATA,  CRLF,        NORMAL_DATA,	// ESCAPING
		QUOTE_DATA,  ESCAPING,    QUOTE_DATA,  QUOTE_DATA,	// UNQUOTE
	};

	if (c == _T(','))       iChar = 0;
	else if (c == _T('\"')) iChar = 1;
	else if (c == _T('\r')) iChar = 2;
	else if (c == _T('\n')) iChar = 2;
	else                    iChar = 3;

	stmt = StateTable[stmt][iChar];
}