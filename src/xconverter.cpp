#include "xconverter.h"

CXConverter::SOption CXConverter::s_Option[] =
{
	{ nullptr, 0, USTR("action:") },
	{ USTR("convert"), USTR('c'), USTR("convert the target file") },
	{ USTR("sample"), 0, USTR("show the samples") },
	{ USTR("help"), USTR('h'), USTR("show this help") },
	{ USTR("file"), USTR('f'), USTR("the target file, required") },
	{ USTR("verbose"), USTR('v'), USTR("show the info") },
	{ USTR("input-code-page"), 0, USTR("[65001|1200|936|932|...]\n\t\tthe code page for read the target file, Windows only") },
	{ USTR("input-code-name"), 0, USTR("[UTF-8|UTF-16LE|CP936|CP932|...]\n\t\tthe code name for read the target file, non-Windows only") },
	{ USTR("output-code-page"), 0, USTR("[65001|1200|936|932|...]\n\t\tthe code page for write the target file, Windows only") },
	{ USTR("output-code-name"), 0, USTR("[UTF-8|UTF-16LE|CP936|CP932|...]\n\t\tthe code name for write the target file, non-Windows only") },
	{ USTR("output-with-bom"), 0, USTR("write BOM for some unicode file") },
	{ nullptr, 0, nullptr }
};

CXConverter::CXConverter()
	: m_eAction(kActionNone)
	, m_bVerbose(false)
	, m_eInputCodeType(kCodeTypeNone)
	, m_nInputCodePage(-1)
	, m_eOutputCodeType(kCodeTypeNone)
	, m_nOutputCodePage(-1)
	, m_bOutputWithBOM(false)
{
}

CXConverter::~CXConverter()
{
}

int CXConverter::ParseOptions(int a_nArgc, UChar* a_pArgv[])
{
	if (a_nArgc <= 1)
	{
		return 1;
	}
	for (int i = 1; i < a_nArgc; i++)
	{
		int nArgpc = static_cast<int>(UCslen(a_pArgv[i]));
		if (nArgpc == 0)
		{
			continue;
		}
		int nIndex = i;
		if (a_pArgv[i][0] != USTR('-'))
		{
			UPrintf(USTR("ERROR: illegal option\n\n"));
			return 1;
		}
		else if (nArgpc > 1 && a_pArgv[i][1] != USTR('-'))
		{
			for (int j = 1; j < nArgpc; j++)
			{
				switch (parseOptions(a_pArgv[i][j], nIndex, a_nArgc, a_pArgv))
				{
				case kParseOptionReturnSuccess:
					break;
				case kParseOptionReturnIllegalOption:
					UPrintf(USTR("ERROR: illegal option\n\n"));
					return 1;
				case kParseOptionReturnNoArgument:
					UPrintf(USTR("ERROR: no argument\n\n"));
					return 1;
				case kParseOptionReturnUnknownArgument:
					UPrintf(USTR("ERROR: unknown argument \"%") PRIUS USTR("\"\n\n"), m_sMessage.c_str());
					return 1;
				case kParseOptionReturnOptionConflict:
					UPrintf(USTR("ERROR: option conflict\n\n"));
					return 1;
				}
			}
		}
		else if (nArgpc > 2 && a_pArgv[i][1] == USTR('-'))
		{
			switch (parseOptions(a_pArgv[i] + 2, nIndex, a_nArgc, a_pArgv))
			{
			case kParseOptionReturnSuccess:
				break;
			case kParseOptionReturnIllegalOption:
				UPrintf(USTR("ERROR: illegal option\n\n"));
				return 1;
			case kParseOptionReturnNoArgument:
				UPrintf(USTR("ERROR: no argument\n\n"));
				return 1;
			case kParseOptionReturnUnknownArgument:
				UPrintf(USTR("ERROR: unknown argument \"%") PRIUS USTR("\"\n\n"), m_sMessage.c_str());
				return 1;
			case kParseOptionReturnOptionConflict:
				UPrintf(USTR("ERROR: option conflict\n\n"));
				return 1;
			}
		}
		i = nIndex;
	}
	return 0;
}

int CXConverter::CheckOptions()
{
	if (m_eAction == kActionNone)
	{
		UPrintf(USTR("ERROR: nothing to do\n\n"));
		return 1;
	}
	if (m_eAction != kActionSample && m_eAction != kActionHelp && m_sFileName.empty())
	{
		UPrintf(USTR("ERROR: no --file option\n\n"));
		return 1;
	}
	if (m_eAction == kActionConvert)
	{
		if (!checkFileCodeType())
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR("\n\n"), m_sMessage.c_str());
			return 1;
		}
	}
	return 0;
}

int CXConverter::Help()
{
	UPrintf(USTR("xconverter %") PRIUS USTR(" by dnasdw\n\n"), AToU(XCONVERTER_VERSION).c_str());
	UPrintf(USTR("usage: xconverter [option...] [option]...\n\n"));
	UPrintf(USTR("option:\n"));
	SOption* pOption = s_Option;
	while (pOption->Name != nullptr || pOption->Doc != nullptr)
	{
		if (pOption->Name != nullptr)
		{
			UPrintf(USTR("  "));
			if (pOption->Key != 0)
			{
				UPrintf(USTR("-%c,"), pOption->Key);
			}
			else
			{
				UPrintf(USTR("   "));
			}
			UPrintf(USTR(" --%-8") PRIUS, pOption->Name);
			if (UCslen(pOption->Name) >= 8 && pOption->Doc != nullptr)
			{
				UPrintf(USTR("\n%16") PRIUS, USTR(""));
			}
		}
		if (pOption->Doc != nullptr)
		{
			UPrintf(USTR("%") PRIUS, pOption->Doc);
		}
		UPrintf(USTR("\n"));
		pOption++;
	}
	return 0;
}

int CXConverter::Action()
{
	if (m_eAction == kActionConvert)
	{
		if (!convertFile())
		{
			UPrintf(USTR("ERROR: convert file failed\n\n"));
			return 1;
		}
	}
	if (m_eAction == kActionSample)
	{
		return sample();
	}
	if (m_eAction == kActionHelp)
	{
		return Help();
	}
	return 0;
}

CXConverter::EParseOptionReturn CXConverter::parseOptions(const UChar* a_pName, int& a_nIndex, int a_nArgc, UChar* a_pArgv[])
{
	if (UCscmp(a_pName, USTR("convert")) == 0)
	{
		if (m_eAction == kActionNone)
		{
			m_eAction = kActionConvert;
		}
		else if (m_eAction != kActionConvert && m_eAction != kActionHelp)
		{
			return kParseOptionReturnOptionConflict;
		}
	}
	else if (UCscmp(a_pName, USTR("sample")) == 0)
	{
		if (m_eAction == kActionNone)
		{
			m_eAction = kActionSample;
		}
		else if (m_eAction != kActionSample && m_eAction != kActionHelp)
		{
			return kParseOptionReturnOptionConflict;
		}
	}
	else if (UCscmp(a_pName, USTR("help")) == 0)
	{
		m_eAction = kActionHelp;
	}
	else if (UCscmp(a_pName, USTR("file")) == 0)
	{
		if (a_nIndex + 1 >= a_nArgc)
		{
			return kParseOptionReturnNoArgument;
		}
		m_sFileName = a_pArgv[++a_nIndex];
	}
	else if (UCscmp(a_pName, USTR("verbose")) == 0)
	{
		m_bVerbose = true;
	}
	else if (UCscmp(a_pName, USTR("input-code-page")) == 0)
	{
		if (a_nIndex + 1 >= a_nArgc)
		{
			return kParseOptionReturnNoArgument;
		}
		UString sInputCodePage = a_pArgv[++a_nIndex];
		m_nInputCodePage = SToN32(sInputCodePage);
	}
	else if (UCscmp(a_pName, USTR("input-code-name")) == 0)
	{
		if (a_nIndex + 1 >= a_nArgc)
		{
			return kParseOptionReturnNoArgument;
		}
		m_sInputCodeName = a_pArgv[++a_nIndex];
	}
	else if (UCscmp(a_pName, USTR("output-code-page")) == 0)
	{
		if (a_nIndex + 1 >= a_nArgc)
		{
			return kParseOptionReturnNoArgument;
		}
		UString sOutputCodePage = a_pArgv[++a_nIndex];
		m_nOutputCodePage = SToN32(sOutputCodePage);
	}
	else if (UCscmp(a_pName, USTR("output-code-name")) == 0)
	{
		if (a_nIndex + 1 >= a_nArgc)
		{
			return kParseOptionReturnNoArgument;
		}
		m_sOutputCodeName = a_pArgv[++a_nIndex];
	}
	else if (UCscmp(a_pName, USTR("output-with-bom")) == 0)
	{
		m_bOutputWithBOM = true;
	}
	return kParseOptionReturnSuccess;
}

CXConverter::EParseOptionReturn CXConverter::parseOptions(int a_nKey, int& a_nIndex, int m_nArgc, UChar* a_pArgv[])
{
	for (SOption* pOption = s_Option; pOption->Name != nullptr || pOption->Key != 0 || pOption->Doc != nullptr; pOption++)
	{
		if (pOption->Key == a_nKey)
		{
			return parseOptions(pOption->Name, a_nIndex, m_nArgc, a_pArgv);
		}
	}
	return kParseOptionReturnIllegalOption;
}

bool CXConverter::checkFileCodeType()
{
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
	if (m_nInputCodePage == 65001)
	{
		m_eInputCodeType = kCodeTypeUTF8;
	}
	else if (m_nInputCodePage == 1200)
	{
		m_eInputCodeType = kCodeTypeUTF16LE;
	}
	else if (m_nInputCodePage == 1201)
	{
		m_eInputCodeType = kCodeTypeUTF16BE;
	}
	else
	{
		m_eInputCodeType = kCodeTypeANSI;
	}
#else
	if (m_sInputCodeName == USTR("UTF-8"))
	{
		m_eInputCodeType = kCodeTypeUTF8;
	}
	else if (m_sInputCodeName == USTR("UTF-16LE"))
	{
		m_eInputCodeType = kCodeTypeUTF16LE;
	}
	else if (m_sInputCodeName == USTR("UTF-16BE"))
	{
		m_eInputCodeType = kCodeTypeUTF16BE;
	}
	else
	{
		m_eInputCodeType = kCodeTypeANSI;
	}
#endif
	FILE* fp = UFopen(m_sFileName.c_str(), USTR("rb"));
	if (fp == nullptr)
	{
		return false;
	}
	Fseek(fp, 0, SEEK_END);
	size_t uFileSize = static_cast<size_t>(Ftell(fp));
	Fseek(fp, 0, SEEK_SET);
	if (m_eInputCodeType == kCodeTypeUTF16LE || m_eInputCodeType == kCodeTypeUTF16BE)
	{
		if (uFileSize % 2 != 0)
		{
			fclose(fp);
			m_sMessage = USTR("not UTF-16 file");
			return false;
		}
		uFileSize /= 2;
		Char16_t* pTemp = new Char16_t[uFileSize + 1];
		fread(pTemp, 2, uFileSize, fp);
		pTemp[uFileSize] = 0;
		if (m_eInputCodeType == kCodeTypeUTF16BE)
		{
			for (size_t i = 0; i < uFileSize; i++)
			{
				pTemp[i] = SDW_CONVERT_ENDIAN16(pTemp[i]);
			}
		}
		if (pTemp[0] == 0xFEFF)
		{
			m_sText = U16ToW(pTemp + 1);
		}
		else
		{
			m_sText = U16ToW(pTemp);
		}
		delete[] pTemp;
	}
	else
	{
		char* pTemp = new char[uFileSize + 1];
		fread(pTemp, 1, uFileSize, fp);
		pTemp[uFileSize] = 0;
		if (m_eInputCodeType == kCodeTypeUTF8)
		{
			if (memcmp(pTemp, "\xEF\xBB\xBF", strlen("\xEF\xBB\xBF")) == 0)
			{
				m_sText = U8ToW(pTemp + 3);
			}
			else
			{
				m_sText = U8ToW(pTemp);
			}
		}
		else
		{
			m_sText = XToW(pTemp, m_nInputCodePage, UToA(m_sInputCodeName).c_str());
		}
		delete[] pTemp;
	}
	fclose(fp);
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
	if (m_nOutputCodePage == 65001)
	{
		m_eOutputCodeType = kCodeTypeUTF8;
	}
	else if (m_nOutputCodePage == 1200)
	{
		m_eOutputCodeType = kCodeTypeUTF16LE;
	}
	else if (m_nOutputCodePage == 1201)
	{
		m_eOutputCodeType = kCodeTypeUTF16BE;
	}
	else
	{
		m_eOutputCodeType = kCodeTypeANSI;
	}
#else
	if (m_sOutputCodeName == USTR("UTF-8"))
	{
		m_eOutputCodeType = kCodeTypeUTF8;
	}
	else if (m_sOutputCodeName == USTR("UTF-16LE"))
	{
		m_eOutputCodeType = kCodeTypeUTF16LE;
	}
	else if (m_sOutputCodeName == USTR("UTF-16BE"))
	{
		m_eOutputCodeType = kCodeTypeUTF16BE;
	}
	else
	{
		m_eOutputCodeType = kCodeTypeANSI;
	}
#endif
	return true;
}

bool CXConverter::convertFile()
{
	string sTextA;
	wstring sTextW;
	if (m_bOutputWithBOM)
	{
		switch (m_eOutputCodeType)
		{
		case kCodeTypeUTF8:
			sTextA.append("\xEF\xBB\xBF");
			break;
		case kCodeTypeUTF16LE:
		case kCodeTypeUTF16BE:
			sTextW.append(L"\uFEFF");
			break;
		default:
			break;
		}
	}
	switch (m_eOutputCodeType)
	{
	case kCodeTypeUTF8:
		sTextA.append(WToU8(m_sText));
		break;
	case kCodeTypeUTF16LE:
	case kCodeTypeUTF16BE:
		sTextW.append(m_sText);
		break;
	default:
		sTextA.append(WToX(m_sText, m_nOutputCodePage, UToA(m_sOutputCodeName).c_str()));
		break;
	}
	if (m_eOutputCodeType == kCodeTypeUTF16BE)
	{
		for (size_t i = 0; i < sTextW.size(); i++)
		{
			sTextW[i] = SDW_CONVERT_ENDIAN16(sTextW[i]);
		}
	}
	FILE* fp = UFopen(m_sFileName.c_str(), USTR("wb"));
	if (fp == nullptr)
	{
		return false;
	}
	switch (m_eOutputCodeType)
	{
	case kCodeTypeUTF16LE:
	case kCodeTypeUTF16BE:
		fwrite(sTextW.c_str(), 2, sTextW.size(), fp);
		break;
	default:
		fwrite(sTextA.c_str(), 1, sTextA.size(), fp);
		break;
	}
	fclose(fp);
	return true;
}

int CXConverter::sample()
{
	UPrintf(USTR("sample:\n"));
	UPrintf(USTR("# convert txt from CP932 to UTF-16LE\n"));
	UPrintf(USTR("xconverter -cvf text.txt --input-code-page 932 --output-code-page 1200\n\n"));
	UPrintf(USTR("# convert txt from CP932 to UTF-16LE with BOM\n"));
	UPrintf(USTR("xconverter -cvf text.txt --input-code-page 932 --output-code-page 1200 --output-with-bom\n\n"));
	return 0;
}

int UMain(int argc, UChar* argv[])
{
	CXConverter xconverter;
	if (xconverter.ParseOptions(argc, argv) != 0)
	{
		return xconverter.Help();
	}
	if (xconverter.CheckOptions() != 0)
	{
		return 1;
	}
	return xconverter.Action();
}
