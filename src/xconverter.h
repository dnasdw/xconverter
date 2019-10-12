#ifndef XCONVERTER_H_
#define XCONVERTER_H_

#include <sdw.h>

class CXConverter
{
public:
	enum EParseOptionReturn
	{
		kParseOptionReturnSuccess,
		kParseOptionReturnIllegalOption,
		kParseOptionReturnNoArgument,
		kParseOptionReturnUnknownArgument,
		kParseOptionReturnOptionConflict
	};
	enum EAction
	{
		kActionNone,
		kActionConvert,
		kActionSample,
		kActionHelp
	};
	enum ECodeType
	{
		kCodeTypeNone,
		kCodeTypeANSI,
		kCodeTypeUTF8,
		kCodeTypeUTF16LE,
		kCodeTypeUTF16BE
	};
	struct SOption
	{
		const UChar* Name;
		int Key;
		const UChar* Doc;
	};
	CXConverter();
	~CXConverter();
	int ParseOptions(int a_nArgc, UChar* a_pArgv[]);
	int CheckOptions();
	int Help();
	int Action();
	static SOption s_Option[];
private:
	EParseOptionReturn parseOptions(const UChar* a_pName, int& a_nIndex, int a_nArgc, UChar* a_pArgv[]);
	EParseOptionReturn parseOptions(int a_nKey, int& a_nIndex, int a_nArgc, UChar* a_pArgv[]);
	bool checkFileCodeType();
	bool convertFile();
	int sample();
	EAction m_eAction;
	UString m_sFileName;
	bool m_bVerbose;
	ECodeType m_eInputCodeType;
	int m_nInputCodePage;
	UString m_sInputCodeName;
	ECodeType m_eOutputCodeType;
	int m_nOutputCodePage;
	UString m_sOutputCodeName;
	bool m_bOutputWithBOM;
	UString m_sMessage;
	wstring m_sText;
};

#endif	// XCONVERTER_H_
