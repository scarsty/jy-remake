//=======================================================================================
// 字符串内码转换
//
// 用GNU的lib-iconv转换有问题，所以就有了这些代码，网上摘抄的，用着不错，谢谢作者。
//
// 原作者：ccrun(老妖) info@ccrun.com
// 本文转自 C++Builder 研究 - http://www.ccrun.com/article/go.asp?i=634&d=04g63p
//
// 代码原来是用于 C++ Builder 的，我只是把它修改成了 C 代码。
//=======================================================================================

#include <windows.h>
#include <tchar.h>

//=======================================================================================
// 大五码转GBK
//=======================================================================================

void BIG52GBK(char *szBuf)
{
	int nStrLen;
	BOOL bValue = FALSE;
	int nReturn;
	wchar_t *pws = NULL;

	if (!strcmp(szBuf, ""))
		return;
	nStrLen = strlen(szBuf);
	pws = (wchar_t *) malloc(sizeof(wchar_t) * (nStrLen + 1));	//new wchar_t[nStrLen + 1];
	nReturn = MultiByteToWideChar(950, 0, szBuf, nStrLen, pws, nStrLen + 1);
	nReturn = WideCharToMultiByte(936, 0, pws, nReturn, szBuf, nStrLen + 1, "?", &bValue);
	szBuf[nReturn] = 0;
	free(pws);
}


//=======================================================================================
// GBK转大五码
// 中華人民共和國 --> い地チ㎝瓣
//=======================================================================================

void GBK2BIG5(char *szBuf)
{
	int nStrLen;
	wchar_t *pws;
	BOOL bValue = FALSE;

	if (!strcmp(szBuf, ""))
		return;
	nStrLen = strlen(szBuf);
	pws = (wchar_t *) malloc(sizeof(wchar_t) * (nStrLen + 1));	//new wchar_t[nStrLen + 1];
	MultiByteToWideChar(936, 0, szBuf, nStrLen, pws, nStrLen + 1);
	WideCharToMultiByte(950, 0, pws, nStrLen, szBuf, nStrLen + 1, "?", &bValue);
	szBuf[nStrLen] = 0;
	free(pws);
}


//=======================================================================================
// 抱歉，这个提示又来了，为了防止不负责任的转载者，只好在此留些信息。
// 作者：ccrun(老妖) info@ccrun.com
// 本文转自 C++Builder 研究 - http://www.ccrun.com/article/go.asp?i=634&d=04g63p
//---------------------------------------------------------------------------
// GB2312码转GBK码
// 本文转自 C++Builder研究 - http://www.ccrun.com/article.asp?i=634&d=04g63p
// 中华人民共和国 --> 中華人民共和國
//=======================================================================================

void GB2GBK(char *szBuf)
{
	DWORD wLCID;
	int nStrLen;
	char *pcBuf;
	int nReturn;

	if (!strcmp(szBuf, ""))
		return;
	nStrLen = strlen(szBuf);
	wLCID = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_PRC);
	nReturn = LCMapString(wLCID, LCMAP_TRADITIONAL_CHINESE, szBuf, nStrLen, NULL, 0);
	if (!nReturn)
		return;
	pcBuf = (char *) malloc(nReturn + 1);	//new char[nReturn + 1];
	wLCID = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_PRC);
	LCMapString(wLCID, LCMAP_TRADITIONAL_CHINESE, szBuf, nReturn, pcBuf, nReturn + 1);
	strncpy(szBuf, pcBuf, nReturn);

	free(pcBuf);
}


//=======================================================================================
// GBK码转GB2312码
// 中華人民共和國 --> 中华人民共和国
//=======================================================================================

void GBK2GB(char *szBuf)
{
	int nStrLen;
	int nReturn;
	char *pcBuf;
	DWORD wLCID;

	if (!strcmp(szBuf, ""))
		return;
	nStrLen = strlen(szBuf);
	wLCID = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_BIG5);
	nReturn = LCMapString(wLCID, LCMAP_SIMPLIFIED_CHINESE, szBuf, nStrLen, NULL, 0);
	if (!nReturn)
		return;
	pcBuf = (char *) malloc(nReturn + 1);	// new char[nReturn + 1];
	wLCID = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_BIG5);
	LCMapString(wLCID, LCMAP_SIMPLIFIED_CHINESE, szBuf, nReturn, pcBuf, nReturn + 1);
	strncpy(szBuf, pcBuf, nReturn);
	free(pcBuf);				//  delete []pcBuf;
}



void BIG52GB(char *s)
{
	BIG52GBK(s);
	GBK2GB(s);
}

void GB2BIG5(char *s)
{
	GB2GBK(s);
	GBK2BIG5(s);
}
